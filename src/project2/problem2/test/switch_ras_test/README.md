# Race-Averse Scheduler Test

Please see the source code in `./jni/switch_ras_test.c` and `./jni/loop.c`

Type `ndk-build` in shell in `./jni` folder, and you will get two executable files in `./libs/armeabi/switch_task_ras_test` and `./libs/armeabi/loop`.

First you should run `loop`. Then start another terminal to run `switch_task_ras_test`, and try to input loop's pid to change its scheduling policy.