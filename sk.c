#include "timer.h"
#include "play_midi.h"
#include "play_wav.h"
#include "keyboard.h"


#include "render.h"
#include "state_ctrl.h"
#include "keyconfig.h"
#include "log.h"

unsigned char wbuff[8192];              /* aux buffer for midi */

int main(int argc, char *argv[]) {

  log_initialize("my_log.txt");
  timer_init();
  key_config_init();
  play_midi_init();

  if(play_wav_init())
  {
    if(keyboard_initialize())
    {
      if(render_initialize(640, 480, 8))
      {
        if(state_ctrl_initialize())
        {
          while(state_ctrl_step())
          {
          }
          state_ctrl_deinitialize();
        }
        render_deinitialize();
      }
      keyboard_deinitialize();
    }
    play_wav_deinit();
  }
  printf("Exiting....\n");
  log_deinitialize();
  play_midi_deinit();
  return 0;  
}
