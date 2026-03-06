#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <sndfile.h>
#include <pulse/simple.h>
#include <pulse/error.h>
#include <pthread.h>

void play_wav(const char *filename) {
    SF_INFO sfinfo = {0};
    SNDFILE *sf = sf_open(filename, SFM_READ, &sfinfo);
    if (!sf) { fprintf(stderr, "Can't open %s: %s\n", filename, sf_strerror(NULL)); return; }

    pa_sample_spec ss = {
        .format   = PA_SAMPLE_S16LE,
        .rate     = sfinfo.samplerate,
        .channels = sfinfo.channels
    };

    int error;
    pa_simple *pa = pa_simple_new(NULL, "keyclick", PA_STREAM_PLAYBACK,
                                  NULL, "click", &ss, NULL, NULL, &error);
    if (!pa) { fprintf(stderr, "PulseAudio error: %s\n", pa_strerror(error)); sf_close(sf); return; }

    short *buf = malloc(sfinfo.frames * sfinfo.channels * sizeof(short));
    sf_readf_short(sf, buf, sfinfo.frames);
    sf_close(sf);

    pa_simple_write(pa, buf, sfinfo.frames * sfinfo.channels * sizeof(short), &error);
    pa_simple_drain(pa, &error);
    pa_simple_free(pa);
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