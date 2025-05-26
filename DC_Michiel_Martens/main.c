#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_CONNECTIONS 4
#define MAX_ROOMS 50
#define MAX_NAME_LEN 32

typedef enum { EMPTY, MONSTER, ITEM, TREASURE } ContentType;

typedef struct Monster {
    char name[MAX_NAME_LEN];
    int hp;
    int damage;
} Monster;

typedef struct Item {
    char name[MAX_NAME_LEN];
    int hp_boost;
    int damage_boost;
} Item;

typedef struct Room {
    int id;
    struct Room *connections[MAX_CONNECTIONS];
    int connection_count;
    ContentType content;
    void *content_data;
    int visited;
} Room;

typedef struct Player {
    int hp;
    int damage;
    Room *current_room;
} Player;

Room *rooms[MAX_ROOMS];
Player player;

void use_item(Item *item) {
    printf("Je gebruikt item: %s\n", item->name);
    player.hp += item->hp_boost;
    player.damage += item->damage_boost;
    printf("HP nu: %d, Damage nu: %d\n", player.hp, player.damage);
}

void bitwise_battle(Monster *monster) {
    printf("Er is een %s in de kamer!\n", monster->name);

    while (monster->hp > 0 && player.hp > 0) {
        int seq = rand() % 17;
        printf("Aanval volgorde: ");
        for (int i = 4; i >= 0; --i) {
            int bit = (seq >> i) & 1;
            printf("%d", bit);
            if (bit) {
                monster->hp -= player.damage;
                printf(" -> %s verliest %d hp (%d hp over)\n", monster->name, player.damage, monster->hp);
            } else {
                player.hp -= monster->damage;
                printf(" -> Held verliest %d hp (%d hp over)\n", monster->damage, player.hp);
            }
            if (monster->hp <= 0 || player.hp <= 0) break;
        }
        printf("\n");
    }

    if (player.hp <= 0) {
        printf("De held is gestorven...\n");
        exit(0);
    } else {
        printf("De %s is verslagen!\n", monster->name);
    }
}

void enter_room(Room *room) {
    player.current_room = room;
    printf("De held betreedt kamer %d\n", room->id);
    room->visited = 1;

    switch (room->content) {
        case EMPTY:
            printf("De kamer is leeg.\n");
            break;
        case ITEM: {
            Item *item = (Item *)room->content_data;
            use_item(item);
            free(item);
            break;
        }
        case MONSTER: {
            Monster *monster = (Monster *)room->content_data;
            bitwise_battle(monster);
            free(monster);
            break;
        }
        case TREASURE:
            printf("Je hebt de schat gevonden! Je wint!\n");
            exit(0);
    }

    room->content = EMPTY;
    room->content_data = NULL;
}

void connect_rooms(Room *a, Room *b) {
    if (a->connection_count < MAX_CONNECTIONS && b->connection_count < MAX_CONNECTIONS) {
        a->connections[a->connection_count++] = b;
        b->connections[b->connection_count++] = a;
    }
}

void generate_dungeon(int room_count) {
    for (int i = 0; i < room_count; i++) {
        Room *r = malloc(sizeof(Room));
        r->id = i;
        r->connection_count = 0;
        r->visited = 0;
        r->content = EMPTY;
        r->content_data = NULL;
        rooms[i] = r;
    }

    for (int i = 1; i < room_count; i++) {
        int prev = rand() % i;
        connect_rooms(rooms[i], rooms[prev]);
    }

    for (int i = 0; i < room_count; i++) {
        while (rooms[i]->connection_count < 2) {
            int other = rand() % room_count;
            if (other != i)
                connect_rooms(rooms[i], rooms[other]);
        }
    }

    int treasure_index = rand() % room_count;
    rooms[treasure_index]->content = TREASURE;

    for (int i = 0; i < room_count; i++) {
        if (rooms[i]->content == TREASURE) continue;
        int roll = rand() % 3;
        if (roll == 0) {
            Monster *m = malloc(sizeof(Monster));
            strcpy(m->name, (rand() % 2) ? "Goblin" : "Orc");
            m->hp = 5 + rand() % 6;
            m->damage = 1 + rand() % 4;
            rooms[i]->content = MONSTER;
            rooms[i]->content_data = m;
        } else if (roll == 1) {
            Item *it = malloc(sizeof(Item));
            strcpy(it->name, (rand() % 2) ? "Potion" : "Sword");
            it->hp_boost = (strcmp(it->name, "Potion") == 0) ? 5 : 0;
            it->damage_boost = (strcmp(it->name, "Sword") == 0) ? 2 : 0;
            rooms[i]->content = ITEM;
            rooms[i]->content_data = it;
        }
    }

    player.hp = 20;
    player.damage = 3;
    player.current_room = rooms[0];
}

void save_game(const char *filename) {
    FILE *f = fopen(filename, "w");
    fprintf(f, "%d %d %d\n", player.hp, player.damage, player.current_room->id);
    fclose(f);
    printf("Spel opgeslagen.\n");
}

void load_game(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) return;

    int hp, dmg, room_id;
    fscanf(f, "%d %d %d", &hp, &dmg, &room_id);
    player.hp = hp;
    player.damage = dmg;
    player.current_room = rooms[room_id];
    fclose(f);
    printf("Spel geladen.\n");
}

int main() {
    srand(time(NULL));

    printf("Nieuw spel (n) of laden (l)? ");
    char keuze;
    scanf(" %c", &keuze);

    generate_dungeon(20);
    if (keuze == 'l') {
        load_game("save.txt");
    } else {
        enter_room(player.current_room);
    }

    while (1) {
        Room *room = player.current_room;
        printf("Deuren naar kamers: ");
        for (int i = 0; i < room->connection_count; i++) {
            printf("%d ", room->connections[i]->id);
        }
        printf("\nKies kamer-id: ");
        int id;
        scanf("%d", &id);
        for (int i = 0; i < room->connection_count; i++) {
            if (room->connections[i]->id == id) {
                enter_room(room->connections[i]);
                break;
            }
        }

        printf("Wil je opslaan (s) of doorgaan (enter)? ");
        getchar(); // consume newline
        keuze = getchar();
        if (keuze == 's') {
            save_game("save.txt");
        }
    }

    return 0;
}