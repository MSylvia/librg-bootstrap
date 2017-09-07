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
 * (same as on the server)
 */
typedef struct {
    u32 model;
    u32 health;

    b32 alive;

    // zplm_vec3_t direction;
    // zplm_vec3_t speed;

} librg_component(gamedata);

#define MY_SERVER_SECRET 23788787283782378
static b32 client_running;

librg_entity_t local_player;

/**
 * As a client we gonna send secret to connect
 * to the server
 */
void on_connection_request(librg_event_t *event) {
    librg_data_wu64(&event->data, MY_SERVER_SECRET);
}

void on_connect_accepted(librg_event_t *event) {
    librg_attach_gamedata(event->entity, (gamedata_t){0});
    local_player = event->entity; // save our local player
}

/**
 * Will be called when we are getting
 * new created entity in the zone
 *
 * NOTE: it will not be our entity
 */
void on_entity_create(librg_event_t *event) {
    zplm_vec3_t position = librg_fetch_transform(event->entity)->position;

    // call to ingame method of creating object
    librg_log("creating entity with id: %u, of type: %u, on position: %f %f %f \n",
        event->entity, librg_entity_type(event->entity),
        position.x, position.y, position.z
    );

    // now read and attach server's gamedata about entity
    gamedata_t gamedata;
    librg_data_rptr(&event->data, &gamedata, sizeof(gamedata_t));
    librg_attach_gamedata(event->entity, gamedata);
}

/**
 * Will be called when we are getting
 * updates about entity inside the stream zone
 */
void on_entity_update(librg_event_t *event) {
    zplm_vec3_t position = librg_fetch_transform(event->entity)->position;

    // call to ingame method of updating object
    librg_log("updating position for entity with id: %u, of type: %u, to position: %f %f %f \n",
        event->entity, librg_entity_type(event->entity),
        position.x, position.y, position.z
    );

    // read helth server sent us
    u32 new_health = librg_data_ru32(&event->data);

    // and assign it into our gamedata, if it's attached
    gamedata_t *gamedata = librg_fetch_gamedata(event->entity);

    if (gamedata) {
        gamedata->health = new_health;
    }
}

void on_entity_remove(librg_event_t *event) {
    // call to ingame method of destroying object
    librg_log("destroying entity with id: %u\n", event->entity);
}

void game_tick() {
    if (librg_entity_valid(local_player) && librg_has_gamedata(local_player)) {
        gamedata_t *g = librg_fetch_gamedata(local_player);

        // if we somehow got health exactly 15
        // KILL THE SERVER
        if (g->health == 15) {
            librg_send(ON_KILL_SERVER, data, {
                librg_data_wu64(&data, MY_SERVER_SECRET);
            });
        }
    }
}

int main() {
    // initialization
    librg_config_t config = {0};

    config.tick_delay   = 64;
    config.mode         = LIBRG_MODE_CLIENT;
    config.world_size   = zplm_vec2(5000.0f, 5000.0f);

    librg_init(config);

    // adding event handlers
    librg_event_add(LIBRG_CONNECTION_REQUEST, on_connection_request);
    librg_event_add(LIBRG_CONNECTION_ACCEPT, on_connect_accepted);

    librg_event_add(LIBRG_ENTITY_CREATE, on_entity_create);
    librg_event_add(LIBRG_ENTITY_UPDATE, on_entity_update);
    librg_event_add(LIBRG_ENTITY_REMOVE, on_entity_remove);

    // connect to the server
    librg_address_t address = { "localhost", 22331 };
    librg_network_start(address);

    // starting main loop
    client_running = true;
    while (client_running) {
        librg_tick();
        zpl_sleep_ms(1);
        game_tick();
    }

    // disconnection from the server
    // and resource disposal
    librg_network_stop();
    librg_free();
    return 0;
}
