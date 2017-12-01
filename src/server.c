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
static b32 server_running;

/**
 * If client wont send us the right number
 * we can disallow him to event join
 */
void on_connection_request(librg_event_t *event) {
    if (librg_data_ru64(event->data) != MY_SERVER_SECRET) {
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
    librg_entity_t *entity = event->entity;

    // attach some custom data
    player_t *player = zpl_malloc(sizeof(player_t));
    zpl_zero_item(player);

    player->model     = 552;
    player->health    = 1000;
    player->alive     = true;

    player->direction = zplm_vec3(0.0f, 0.0f, 0.0f);
    player->speed     = zplm_vec3(0.0f, 0.0f, 0.0f);

    // set the position
    entity->position  = zplm_vec3(0.0f, 0.0f, 0.0f);
    entity->user_data = (void *)player; /* save custom data inside */

    // and allow player to stream changes of his entity
    // he will be sending position changes and other stuff
    librg_entity_control_set(event->ctx, entity->id, entity->client_peer);
}

/**
 * This will be called when player
 * enters zone for this entity for first time
 *
 * we will send one big structure in a single message
 */
void on_entity_create(librg_event_t *event) {
    player_t *player = (player_t *)event->entity->user_data;
    librg_data_wptr(event->data, player, sizeof(player_t));
}

/**
 * This callback will be called while
 * entity is inside the player stream zone
 *
 * We will be sending only u32 with health
 * (entity position will be sent always)
 */
void on_entity_update(librg_event_t *event) {
    player_t *player = (player_t *)event->entity->user_data;
    librg_data_wu32(event->data, player->health);
}

/**
 * Now, this is not an event, this is a "raw" message
 * However it looks and works similary
 */
void on_kill_server(librg_message_t *msg) {
    if (librg_data_ru64(msg->data) == MY_SERVER_SECRET) {
        server_running = false;
    }
}

void spawn_bot(librg_ctx_t *ctx) {
    librg_entity_id enemyid = librg_entity_create(ctx, 0);
    librg_entity_t *enemy   = librg_entity_fetch(ctx, enemyid);

    player_t *player = zpl_malloc(sizeof(player_t));
    zpl_zero_item(player);

    enemy->user_data = player;

    enemy->position.x = (float)(2000 - rand() % 4000);
    enemy->position.y = (float)(2000 - rand() % 4000);
}

int main() {
    // initialization
    librg_ctx_t ctx = {0};

    ctx.tick_delay   = 32; // 32ms delay, is around 30hz, quite fast
    ctx.mode         = LIBRG_MODE_SERVER;
    ctx.world_size   = zplm_vec3(5000.0f, 5000.0f, 5000.0f);

    librg_init(&ctx);

    // adding event handlers
    librg_event_add(&ctx, LIBRG_CONNECTION_REQUEST, on_connection_request);
    librg_event_add(&ctx, LIBRG_CONNECTION_ACCEPT, on_connect_accepted);

    librg_event_add(&ctx, LIBRG_ENTITY_CREATE, on_entity_create);
    librg_event_add(&ctx, LIBRG_ENTITY_UPDATE, on_entity_update);

    librg_network_add(&ctx, ON_KILL_SERVER, on_kill_server);

    // starting server
    librg_address_t address = {0}; address.port = 22331;
    librg_network_start(&ctx, address);

    // lets also spawn 250 bots :O
    for (int i = 0; i < 2500; ++i) {
        spawn_bot(&ctx);
    }

    // starting main loop
    server_running = true;
    while (server_running) {
        librg_tick(&ctx);
        zpl_sleep_ms(1);
    }

    // stopping network and resources disposal
    librg_network_stop(&ctx);
    librg_free(&ctx);
    return 0;
}
