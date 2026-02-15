#ifndef CRE_SCENES_H
#define CRE_SCENES_H

// Forward Declaration
typedef struct EntityRegistry EntityRegistry;
typedef struct CommandBus CommandBus;

typedef struct {
    void (*Init)(EntityRegistry* reg, CommandBus* bus);
    void (*Update)(EntityRegistry* reg, CommandBus* bus,float dt);
    void (*Draw)(EntityRegistry* reg, CommandBus* bus);
    void (*Unload)(EntityRegistry* reg, CommandBus* bus);
} Scene;

#endif