/*
 * Library to access MPU-401 hardware
 *
 * Copyright (C) 2014-2018 Mateusz Viste
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef mpu401_h_sentinel
#define mpu401_h_sentinel

/* wait until it's okay for us to write to the MPU */
void mpu401_waitwrite(int mpuport);

/* polls the midi interface - returns 0 if nothing is available to be read, non-zero otherwise. note that this should be checked as often as possible - whenever UART have some bytes for you, you MUST read them out */
int mpu401_poll(int mpuport);

void mpu401_waitread(int mpuport);

/* flush everything from the MPU port (if anything) */
void mpu401_flush(int mpuport);

/* resets the MPU-401. returns 0 on success, non-zero otherwise. */
int mpu401_rst(int mpuport);

/* switches the MPU-401 into 'dumb UART' mode */
void mpu401_uart(int mpuport);


void mpu401_ctrl_cmd(int channel, int cmd, int id, int val);
void mpu401_controller(int channel, int id, int val);
void mpu401_clear();
void mpu401_noteoff(int channel, int note);
void mpu401_noteon(int channel, int note, int velocity);
void mpu401_setprog(int channel, int program);
void mpu401_pitchwheel(int channel, int wheelvalue);
void mpu401_chanpressure(int channel, int pressure);
void mpu401_keypressure(int channel, int note, int pressure);
void mpu401_sysex(int channel, unsigned char *buff, int bufflen);


#endif
