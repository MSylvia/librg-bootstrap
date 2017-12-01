#define LIBRG_DEBUG
#define LIBRG_IMPLEMENTATION
#include <librg.h>

enum {
    ON_KILL_SERVER = LIBRG_EVENT_LAST, // we are counting from N events +1
    ON_OTHER_MSG_1,
    ON_OTHER_MSG_2,
};

/**
 * This sturcture defines our nice component
 * (same as on the client)
 */
typedef struct {
    u32 model;
    u32 health;

    b32 alive;

    zplm_vec3_t direction;
    zplm_vec3_t speed;
} player_t;

#define MY_SERVER_SECRET 23788787283782378
static b32 client_running;

librg_entity_t *local_player;

/**
 * As a client we gonna send secret to connect
 * to the server
 */
void on_connection_request(librg_event_t *event) {
    librg_data_wu64(event->data, MY_SERVER_SECRET);
}

void on_connect_accepted(librg_event_t *event) {
    // attach some custom data
    player_t *player = zpl_malloc(sizeof(player_t));
    zpl_zero_item(player); /* fill it with zeros */
    event->entity->user_data = (void *)player;

    local_player = event->entity; // save our local player
}

/**
 * Will be called when we are getting
 * new created entity in the zone
 *
 * NOTE: it will not be our entity
 */
void on_entity_create(librg_event_t *event) {
    // call to ingame method of creating object
    librg_log("creating entity with id: %u, of type: %u, on position: %f %f %f \n",
        event->entity->id,
        event->entity->type,
        event->entity->position.x,
        event->entity->position.y,
        event->entity->position.z
    );

    // now read and attach server's player_t about entity
    event->entity->user_data = zpl_malloc(sizeof(player_t));
    librg_data_rptr(event->data, event->entity->user_data, sizeof(player_t));
}

/**
 * Will be called when we are getting
 * updates about entity inside the stream zone
 */
void on_entity_update(librg_event_t *event) {
    // call to ingame method of updating object
    librg_log("updating position for entity with id: %u, of type: %u, to position: %f %f %f \n",
        event->entity->id,
        event->entity->type,
        event->entity->position.x,
        event->entity->position.y,
        event->entity->position.z
    );

    // read helth server sent us
    u32 new_health = librg_data_ru32(event->data);

    // and assign it into our player data, if it's attached
    player_t *player = (player_t *)event->entity->user_data;

    if (player) {
        player->health = new_health;
    }
}

void on_entity_remove(librg_event_t *event) {
    // call to ingame method of destroying object
    librg_log("destroying entity with id: %u\n", event->entity->id);
    zpl_mfree(event->entity->user_data); /* deallocating custom data */
}

void game_tick(librg_ctx_t *ctx) {
    if (local_player && local_player->user_data) {
        player_t *player = (player_t *)local_player->user_data;

        // if we somehow got health exactly 15
        // KILL THE SERVER
        if (player->health == 15) {
            librg_send(ctx, ON_KILL_SERVER, data, {
                librg_data_wu64(&data, MY_SERVER_SECRET);
            });
        }
    }
}

int main() {
    // initialization
    librg_ctx_t ctx = {0};

    ctx.tick_delay   = 64;
    ctx.mode         = LIBRG_MODE_CLIENT;
    ctx.world_size   = zplm_vec3(5000.0f, 5000.0f, 5000.0f);

    librg_init(&ctx);

    // adding event handlers
    librg_event_add(&ctx, LIBRG_CONNECTION_REQUEST, on_connection_request);
    librg_event_add(&ctx, LIBRG_CONNECTION_ACCEPT, on_connect_accepted);

    librg_event_add(&ctx, LIBRG_ENTITY_CREATE, on_entity_create);
    librg_event_add(&ctx, LIBRG_ENTITY_UPDATE, on_entity_update);
    librg_event_add(&ctx, LIBRG_ENTITY_REMOVE, on_entity_remove);

    // connect to the server
    librg_address_t address = { 22331, "localhost" };
    librg_network_start(&ctx, address);

    // starting main loop
    client_running = true;
    while (client_running) {
        librg_tick(&ctx);
        game_tick(&ctx);
        zpl_sleep_ms(1);
    }

    // disconnection from the server
    // and resource disposal
    librg_network_stop(&ctx);
    librg_free(&ctx);
    return 0;
}
