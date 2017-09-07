#define LIBRG_DEBUG
#define LIBRG_IMPLEMENTATION
#include <librg.h>

enum {
    ON_KILL_SERVER = LIBRG_LAST_ENUM_NUMBER, // we are counting from N events +1
    ON_OTHER_MSG_1,
    ON_OTHER_MSG_2,
};

/**
 * This sturcture defines our nice component
 */
typedef struct {
    u32 model;
    u32 health;
    b32 alive;
} librg_component(gamedata);

#define MY_SERVER_SECRET 23788787283782378
static b32 server_running;

/**
 * If client wont send us the right number
 * we can disallow him to event join
 */
void on_connection_request(librg_event_t *event) {
    if (librg_data_ru64(&event->data) != MY_SERVER_SECRET) {
        // reject him from joining the server
        librg_event_reject(event);
    }
}

/**
 * Attach new component onto our new player
 * and put it some data
 */
void on_connect_accepted(librg_event_t *event) {
    librg_log("someone connected to the server!\n");

    // we have some default components already in place
    librg_transform_t *transform = librg_fetch_transform(event->entity);
    librg_client_t    *client    = librg_fetch_client(event->entity);

    gamedata_t *gamedata = librg_attach_gamedata(
        event->entity, (gamedata_t){0}
    );

    gamedata->model     = 552;
    gamedata->health    = 1000;
    gamedata->alive     = true;

    // set the position
    transform->position = zplm_vec3(0.0f, 0.0f, 0.0f);

    // and allow player to stream changes of his entity
    // he will be sending position changes and other stuff
    librg_streamer_client_set(event->entity, client->peer);
}

/**
 * This will be called when player
 * enters zone for this entity for first time
 *
 * we will send one big structure in a single message
 */
void on_entity_create(librg_event_t *event) {
    librg_data_wptr(&event->data, librg_fetch_gamedata(event->entity), sizeof(gamedata_t));
}

/**
 * This callback will be called while
 * entity is inside the player stream zone
 *
 * We will be sending only u32 with health
 * (librg_transform_t will be sent always)
 */
void on_entity_update(librg_event_t *event) {
    librg_data_wu32(&event->data, librg_fetch_gamedata(event->entity)->health);
}

/**
 * Now, this is not an event, this is a "raw" message
 * However it looks and works similary
 */
void on_kill_server(librg_message_t *msg) {
    if (librg_data_ru64(&msg->data) == MY_SERVER_SECRET) {
        server_running = false;
    }
}

void spawn_bot() {
    librg_entity_t enemy = librg_entity_create(0);
    librg_attach_gamedata(enemy, (gamedata_t){0});

    librg_transform_t *transform = librg_fetch_transform(enemy);
    transform->position.x = (float)(2000 - rand() % 4000);
    transform->position.y = (float)(2000 - rand() % 4000);
}

int main() {
    // initialization
    librg_config_t config = {0};

    config.tick_delay   = 32; // 32ms delay, is around 30hz, quite fast
    config.mode         = LIBRG_MODE_SERVER;
    config.world_size   = zplm_vec2(5000.0f, 5000.0f);

    librg_init(config);

    // adding event handlers
    librg_event_add(LIBRG_CONNECTION_REQUEST, on_connection_request);
    librg_event_add(LIBRG_CONNECTION_ACCEPT, on_connect_accepted);

    librg_event_add(LIBRG_ENTITY_CREATE, on_entity_create);
    librg_event_add(LIBRG_ENTITY_UPDATE, on_entity_update);

    librg_network_add(ON_KILL_SERVER, on_kill_server);

    // starting server
    librg_address_t address = {0}; address.port = 22331;
    librg_network_start(address);

    // lets also spawn 250 bots :O
    for (int i = 0; i < 250; ++i) {
        spawn_bot();
    }

    // starting main loop
    server_running = true;
    while (server_running) {
        librg_tick();
        zpl_sleep_ms(1);
    }

    // stopping network and resources disposal
    librg_network_stop();
    librg_free();
    return 0;
}
