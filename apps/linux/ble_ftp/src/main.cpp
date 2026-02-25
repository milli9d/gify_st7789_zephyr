#include "simpleble/Peripheral.h"
#include "simpleble/Service.h"
#include "simpleble/Types.h"
#include <condition_variable>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <print>
#include <span>

#include <chrono>
#include <stddef.h>
#include <sys/types.h>
#include <thread>
#include <unordered_map>

#include <simpleble/SimpleBLE.h>
#include <string>
#include <unordered_set>
#include <vector>

#include "this_is_fine.h"

#define GIFY_FTP_SERVICE_UUID "deadd00d-0000-0000-dada-deeddead0000"

#define GIFY_FTP_CHAR_OPEN_FILE_UUID "deadd00d-0000-0001-dada-deeddead0000"
#define GIFY_FTP_CHAR_CLOSE_FILE_UUID "deadd00d-0000-0002-dada-deeddead0000"
#define GIFY_FTP_CHAR_WRITE_CHUNK_UUID "deadd00d-0000-0003-dada-deeddead0000"

std::unordered_map<SimpleBLE::BluetoothAddress, SimpleBLE::Peripheral>
    _gify_devices{};

typedef struct ble_ftp_ctx_t {
  SimpleBLE::Peripheral peripheral;
  SimpleBLE::Service ftp_service;
  SimpleBLE::Characteristic open_file_char;
  SimpleBLE::Characteristic close_file_char;
  bool initialized;
} ble_ftp_ctx_t;

ble_ftp_ctx_t ftp_ctx{};

auto adapter = SimpleBLE::Adapter::get_adapters()[0];

static std::mutex ftp_mutex;
static std::condition_variable ftp_cv;

static void progress_bar(size_t current, size_t total) {
  const int bar_width = 50;
  float progress = static_cast<float>(current) / static_cast<float>(total);
  int pos = static_cast<int>(bar_width * progress);

  std::cout << "[";
  for (int i = 0; i < bar_width; ++i) {
    if (i < pos)
      std::cout << "=";
    else if (i == pos)
      std::cout << ">";
    else
      std::cout << " ";
  }
  std::cout << "] " << int(progress * 100.0) << " % [ " << current << " / " << total <<" bytes ]\r";
  std::cout.flush();
}

static void init_ble_gap() {
  adapter.set_callback_on_scan_start([]() { std::print("Scan started\n"); });
  adapter.set_callback_on_scan_stop([]() { std::print("Scan stopped\n"); });
  adapter.set_callback_on_scan_found([](SimpleBLE::Peripheral peripheral) {
    SimpleBLE::BluetoothAddress address = peripheral.address();

    for (const auto &[id, data] : peripheral.manufacturer_data()) {
      if (id == 0xDEAD) {
        if (std::string(data) == "GIFY") {
          std::print("Found device: {} RSSI : {} dBm\n",
                     peripheral.address().c_str(), peripheral.rssi());
        }
        _gify_devices.insert({address, peripheral});
        break;
      }
    }
  });
}

static void print_usage(const char *prog) {
  std::cout << "Usage: " << prog << " --file PATH | -f PATH [or positional PATH]\n";
}

struct CliOptions {
  std::string file_path;
  bool show_help = false;
};

static CliOptions parse_args(int argc, char **argv) {
  CliOptions opts;
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "-h" || arg == "--help") {
      opts.show_help = true;
      break;
    } else if (arg == "-f" || arg == "--file") {
      if (i + 1 < argc) {
        opts.file_path = argv[++i];
      } else {
        std::print("Missing value for {}\n", arg.c_str());
        opts.show_help = true;
        break;
      }
    } else if (!arg.empty() && arg[0] != '-') {
      // Positional file path if not set yet
      if (opts.file_path.empty()) {
        opts.file_path = arg;
      } else {
        std::print("Unexpected positional argument: {}\n", arg.c_str());
        opts.show_help = true;
        break;
      }
    } else {
      std::print("Unknown option: {}\n", arg.c_str());
      opts.show_help = true;
      break;
    }
  }
  return opts;
}

int main(int argc, char **argv) {
  std::print("Hello BLE\n");

  // Parse arguments
  CliOptions opts = parse_args(argc, argv);
  if (opts.show_help) {
    print_usage(argv[0]);
    return 1;
  }
  std::string filename;
  if (!opts.file_path.empty()) {
    filename = opts.file_path;
    std::print("Using file path: {}\n", filename.c_str());
  } else {
    std::print("Error: file_path is required.\n");
    print_usage(argv[0]);
    return 1;
  }

  init_ble_gap();

  auto scan_start = std::chrono::steady_clock::now();
  adapter.scan_start();
  while (1) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    if (std::chrono::steady_clock::now() - scan_start >
        std::chrono::seconds(5)) {
      adapter.scan_stop();
      break;
    }
  }

  /* attempt to send file to found devices */
  if (_gify_devices.empty()) {
    std::print("No GIFY devices found during scan.\n");
    return 1;
  }

  SimpleBLE::Peripheral target =
      _gify_devices.extract(_gify_devices.begin()).mapped();

  target.set_callback_on_connected([&target]() {
    std::print("Peripheral connected callback invoked.\n");
    for (auto service : target.services()) {
      std::print("Service UUID: {}\n", service.uuid().c_str());
      if (service.uuid() == GIFY_FTP_SERVICE_UUID) {
        std::print("Found GIFY FTP Service!\n");
        ftp_ctx.ftp_service = service;
        ftp_ctx.peripheral = target;
      } else {
        continue;
      }

      for (auto characteristic : service.characteristics()) {
        std::print("\tCharacteristic UUID: {}\n",
                   characteristic.uuid().c_str());
        if (characteristic.uuid() == GIFY_FTP_CHAR_OPEN_FILE_UUID) {
          std::print("\tFound Open File Characteristic!\n");
          ftp_ctx.open_file_char = characteristic;
        } else if (characteristic.uuid() == GIFY_FTP_CHAR_CLOSE_FILE_UUID) {
          std::print("\tFound Close File Characteristic!\n");
          ftp_ctx.close_file_char = characteristic;
        }
      }

      ftp_ctx.initialized = true;
      ftp_cv.notify_one();
    }
  });

  target.connect();
  if (target.is_connected()) {
    std::print("Connected to {}\n", target.address().c_str());
  } else {
    std::print("Failed to connect to {}\n", target.address().c_str());
    return 1;
  }

  // Wait for services/characteristics discovery via the connected callback.
  {
    std::unique_lock<std::mutex> lk(ftp_mutex);
    ftp_cv.wait_for(lk, std::chrono::seconds(3),
                    [] { return ftp_ctx.initialized; });
  }
  if (!ftp_ctx.initialized) {
    std::print("FTP context not initialized properly (timeout waiting for "
               "discovery).\n");
    return 1;
  }

  // Validate file path before opening
  if (!std::filesystem::exists(filename)) {
    std::print("Provided file_path does not exist: {}\n", filename.c_str());
    return 1;
  }
  if (!std::filesystem::is_regular_file(filename)) {
    std::print("Provided file_path is not a regular file: {}\n",
               filename.c_str());
    return 1;
  }
  const std::string basename =
      std::filesystem::path(filename).filename().string() +
      std::filesystem::path(".bin").extension().string();

  // memory map the file
  std::ifstream file_stream(filename, std::ios::binary | std::ios::ate);
  if (!file_stream.is_open()) {
    std::print("Failed to open file: {}\n", filename.c_str());
    return 1;
  }

  size_t file_size = file_stream.tellg();
  file_stream.seekg(0, std::ios::beg);

  std::vector<char> file_data(file_size);
  if (!file_stream.read(file_data.data(), file_size)) {
    std::print("Failed to read file: {}\n", filename.c_str());
    return 1;
  }

  std::print("Read file: {} ({} bytes)\n", filename.c_str(), file_size);

  std::vector<uint8_t> open_file_payload;
  // Append file size (4 bytes, little-endian)
  for (size_t i = 0; i < 4; ++i) {
    open_file_payload.push_back(
        static_cast<uint8_t>((file_size >> (i * 8)) & 0xFF));
  }

  // Append file name length (2 bytes, little-endian)
  uint16_t filename_length = static_cast<uint16_t>(basename.size());
  open_file_payload.push_back(static_cast<uint8_t>(filename_length & 0xFF));
  open_file_payload.push_back(
      static_cast<uint8_t>((filename_length >> 8) & 0xFF));

  // Append file name
  open_file_payload.insert(open_file_payload.end(), basename.begin(),
                           basename.end());

  ftp_ctx.peripheral.write_request(
      GIFY_FTP_SERVICE_UUID, GIFY_FTP_CHAR_OPEN_FILE_UUID, open_file_payload);
  std::print("Sent Open File request (write with response).\n");

  uint16_t mtu = ftp_ctx.peripheral.mtu();
  std::print("Peripheral MTU: {} bytes\n", mtu);

  // Base chunk size is MTU minus ATT header (3 bytes), determined by device.
  size_t default_chunk = (mtu > 3) ? (mtu - 3) : mtu;
  std::print("Using chunk size: {} bytes\\n", default_chunk);

  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  size_t bytes_sent = 0;
  while (bytes_sent < file_size) {
    // Re-check MTU in case it changes; recompute effective chunk (MTU-3).
    mtu = ftp_ctx.peripheral.mtu();
    default_chunk = (mtu > 3) ? (mtu - 3) : mtu;
    size_t bytes_to_send = std::min(default_chunk, file_size - bytes_sent);
    std::vector<uint8_t> chunk_data(file_data.data() + bytes_sent,
                                    file_data.data() + bytes_sent +
                                        bytes_to_send);
    ftp_ctx.peripheral.write_request(
        GIFY_FTP_SERVICE_UUID, GIFY_FTP_CHAR_WRITE_CHUNK_UUID, chunk_data);
    bytes_sent += bytes_to_send;
    progress_bar(bytes_sent, file_size);
  }

  std::print("\nFile transfer complete.\n");

  std::vector<uint8_t> close_file_payload;
  close_file_payload.push_back(0x00); // Status: Success
  ftp_ctx.peripheral.write_request(
      GIFY_FTP_SERVICE_UUID, GIFY_FTP_CHAR_CLOSE_FILE_UUID, close_file_payload);
  std::print("Sent Close File request (write with response).\n");

  file_stream.close();

  return 0;
}