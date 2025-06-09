/**
 * @file gif_player.hpp
 * @brief Declaration of the gif_player class for handling GIF playback
 * functionality.
 *
 * This file contains the definition of the gif_player class, which provides
 * an interface for managing and playing GIF animations.
 */

namespace gify {

#define FRAME_RATE 24u
// #define FRAME_RATE 30u
// #define FRAME_RATE 60u

#define TFT_W      240
#define TFT_H      280
#define BPP_COOKED 2u

/**
 * @class gif_player
 * @brief A class for managing and playing GIF animations.
 *
 * The gif_player class provides functionality to load, control, and display
 * GIF animations. It serves as a core component for handling GIF playback
 * within the gify namespace.
 */
class gif_player
{
  private:
    // Private members for internal use.

    static void _run(void* p0, void* p1, void* p2);

  public:
    /**
     * @brief Constructs a new gif_player object.
     *
     * Initializes the gif_player instance and prepares it for use.
     */
    gif_player();

    /**
     * @brief Destroys the gif_player object.
     *
     * Cleans up resources used by the gif_player instance.
     */
    ~gif_player();
};

} // namespace gify
