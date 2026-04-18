#ifndef CRE_ENGINEPACKETS_H
#define CRE_ENGINEPACKETS_H

struct EntityRegistry;
struct CommandBus;
struct TimeContext;

struct p0Packet {
  TimeContext *time;
};
struct p1Packet {
  EntityRegistry *reg;
  CommandBus *bus;
  const TimeContext *time;
};

struct p2Packet {
  EntityRegistry *reg;
  CommandBus *bus;
  TimeContext *time;
};

struct p3Packet {
  EntityRegistry *reg;
  CommandBus *bus;
  const TimeContext *time;
};

struct p4Packet {
  CommandBus *bus;
};

#endif
