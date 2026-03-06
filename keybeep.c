#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <alsa/asoundlib.h>
#include <sndfile.h>
#include <pthread.h>

void play_wav(const char *filename) {
    SF_INFO sfinfo;
    SNDFILE *sf = sf_open(filename, SFM_READ, &sfinfo);
    if (!sf) { fprintf(stderr, "Can't open %s\n", filename); return; }

    snd_pcm_t *pcm;
    snd_pcm_open(&pcm, "default", SND_PCM_STREAM_PLAYBACK, 0);
    snd_pcm_set_params(pcm, SND_PCM_FORMAT_S16_LE,
                       SND_PCM_ACCESS_RW_INTERLEAVED,
                       sfinfo.channels, sfinfo.samplerate, 1, 50000);

    short *buf = malloc(sfinfo.frames * sfinfo.channels * sizeof(short));
    sf_readf_short(sf, buf, sfinfo.frames);
    sf_close(sf);

    snd_pcm_writei(pcm, buf, sfinfo.frames);
    snd_pcm_drain(pcm);
    snd_pcm_close(pcm);
    free(buf);
}

void *beep_thread(void *arg) {
    play_wav("click.wav");
    return NULL;
}

int main(int argc, char *argv[]) {
    const char *dev = argc > 1 ? argv[1] : "/dev/input/event0";
    int fd = open(dev, O_RDONLY);
    if (fd < 0) { perror("open"); return 1; }

    // ioctl(fd, EVIOCGRAB, 1);

    struct input_event ev;
    while (read(fd, &ev, sizeof(ev)) == sizeof(ev)) {
        if (ev.type == EV_KEY && ev.value == 1) {
            pthread_t t;
            pthread_create(&t, NULL, beep_thread, NULL);
            pthread_detach(t);
        }
    }

    close(fd);
    return 0;
}
