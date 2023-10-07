# Tonewheel

Audio engine for making sample-based virtual instruments.

## Engine architecture

```

bus[0]-----------------------------[ FX chain ]-------->

bus[1]-----------------------------[ FX chain ]-------->
             ^                                              Outputs
...          |
             |
bus[N]-------|---------------------[ FX chain ]-------->
             |
             |
             |
        [ FX chain ]
             |
         [ Voice ] - [ envelope ]
             |
         [ Sample ]
```

- All buses and voices are stereo
- Supported sample formats: Wav PCM, Ogg Vorbis
- Triggered voices can be places on any bus (but only one bus)
- Voices can have a dynamic FX chain created upon triggering
- Voice parameters can be modulated using [exprtk](https://www.partow.net/programming/exprtk/index.html) expressions
