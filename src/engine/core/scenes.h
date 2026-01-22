#ifndef SCENES_H
#define SCENES_H

typedef struct {
    void (*Init)(void);
    void (*Update)(void);
    void (*Draw)(void);
    void (*Unload)(void);
} Scene;

#endif