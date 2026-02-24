#include <iostream>
#include <fstream>
#include "json.hpp"
#include <list>
#include <random>
#include <queue>
#include <string>
#include <algorithm>

using json = nlohmann::json;

struct Character {
    std::string name = "", role = "";
    int hp = 0, maxhp = 0, agi = 0;
    bool shielded = false;

    virtual ~Character(){}

    void heal(int amount) {
        hp += amount;
        if (hp > maxhp) hp = maxhp;
    }
 
};

struct PlayerCharacter : public Character {};
struct EnemyCharacter : public Character {};

void Menu() {
    std::cout << "----------RPG Battle----------" << std::endl;
    std::cout << "1. Start" << std::endl;
    std::cout << "2. Quit" << std::endl;
}

std::list<PlayerCharacter> loadCharacters() {
    std::list<PlayerCharacter> characters;
    std::ifstream file("characters.json");
    json j;
    file >> j;

    for (auto& item : j) {
        PlayerCharacter c;
        c.name = item["name"];
        c.role = item["role"];
        c.hp = item["hp"];
        c.maxhp = c.hp;
        c.agi = item["agi"];
        characters.push_back(c);
    }
    return characters;
}

std::list<EnemyCharacter> loadEnemies() {
    std::list<EnemyCharacter> enemies;
    std::ifstream file("enemies.json");
    json j;
    file >> j;

    for (auto& item : j) {
        EnemyCharacter e;
        e.name = item["name"];
        e.role = item["role"];
        e.hp = item["hp"];
        e.maxhp = e.hp;
        e.agi = item["agi"];
        enemies.push_back(e);
    }
    return enemies;
}

template <typename T>
void showCharacters(const std::list<T>& characters) {
    int i = 1;
    for (const auto& c : characters) {
        std::cout << i << ". " << c.name << " | Role: " << c.role
            << " | HP: " << c.hp << "/" << c.maxhp
            << " | AGI: " << c.agi << std::endl;
        i++;
    }
}

PlayerCharacter createHero() {
    PlayerCharacter c;
    std::cin.ignore();
    std::cout << "Write your Hero's name: ";
    std::getline(std::cin, c.name);
    std::cout << "Write your Hero's role: ";
    std::getline(std::cin, c.role);

    if (c.role == "Tank") { c.hp = c.maxhp = 120; c.agi = 8; }
    else if (c.role == "Healer") { c.hp = c.maxhp = 80; c.agi = 15; }
    else if (c.role == "Attacker") { c.hp = c.maxhp = 90; c.agi = 10; }
    else { c.hp = c.maxhp = 100; c.agi = 10; }

    return c;
}

std::list<EnemyCharacter> getRandomEnemies(std::list<EnemyCharacter> allEnemies) {
    std::list<EnemyCharacter> selected;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distCount(1, 3);
    int count = std::min((int)allEnemies.size(), distCount(gen));

    while (selected.size() < count && !allEnemies.empty()) {
        std::uniform_int_distribution<> distIndex(0, allEnemies.size() - 1);
        int idx = distIndex(gen);
        auto it = allEnemies.begin();
        std::advance(it, idx);
        selected.push_back(*it);
        allEnemies.erase(it);
    }
    return selected;
}

void EnemyTurn(EnemyCharacter& enemy, std::list<EnemyCharacter>& allies, std::list<PlayerCharacter>& players) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> moveDist(1, 2);
    int move = moveDist(gen);

    std::cout << enemy.name << "'s turn!" << std::endl;

    if (enemy.role == "Tank") {
        if (move == 1) {
            int damage = 5;
            for (auto& p : players) {
                if (!p.shielded) { p.hp -= damage; std::cout << p.name << " takes " << damage << " damage!" << std::endl; }
                else { std::cout << p.name << " is shielded! No damage." << std::endl; p.shielded = false; }
            }
        }
        else {
            std::uniform_int_distribution<> targetDist(0, allies.size() - 1);
            int idx = targetDist(gen);
            auto it = allies.begin();
            std::advance(it, idx);
            it->shielded = true;
            std::cout << it->name << " is shielded!" << std::endl;
        }
    }
    else if (enemy.role == "Healer") {
        if (move == 1) {
            enemy.heal(10);
            std::cout << enemy.name << " heals self. HP: " << enemy.hp << "/" << enemy.maxhp << std::endl;
        }
        else {
            std::uniform_int_distribution<> targetDist(0, players.size() - 1);
            int idx = targetDist(gen);
            auto it = players.begin();
            std::advance(it, idx);
            int damage = 8;
            if (!it->shielded) { it->hp -= damage; std::cout << it->name << " takes " << damage << " damage!" << std::endl; }
            else { std::cout << it->name << " is shielded! No damage." << std::endl; it->shielded = false; }

            for (auto& ally : allies) {
                ally.heal(damage / 2);
                std::cout << ally.name << " heals " << damage / 2 << " HP!" << std::endl;
            }
        }
    }
    else if (enemy.role == "Attacker") {
        if (move == 1) {
            std::uniform_int_distribution<> targetDist(0, players.size() - 1);
            int idx = targetDist(gen);
            auto it = players.begin(); std::advance(it, idx);
            int damage = 15;
            if (!it->shielded) { it->hp -= damage; std::cout << it->name << " takes " << damage << " damage!" << std::endl; }
            else { std::cout << it->name << " is shielded! No damage." << std::endl; it->shielded = false; }
        }
        else {
            int damage = 5;
            for (auto& p : players) {
                if (!p.shielded) { p.hp -= damage; std::cout << p.name << " takes " << damage << " damage!" << std::endl; }
                else { std::cout << p.name << " is shielded! No damage." << std::endl; p.shielded = false; }
            }
        }
    }
}

void PlayerTurn(PlayerCharacter& player, std::list<PlayerCharacter>& allies, std::list<EnemyCharacter>& enemies) {
    int Playermove;
    std::cout << player.name << "'s turn!" << std::endl;
    std::cout << "Choose Your Move: " << std::endl;

    if (player.role == "Tank") {
        std::cout << "1. Splash Damage," << std::endl;
        std::cout << "2. Block Damage." << std::endl;
        std::cin >> Playermove;

        if (Playermove == 1) {
            int damage = 5;
            for (auto& enemy : enemies) {
                if (!enemy.shielded) { enemy.hp -= damage; std::cout << enemy.name << " takes " << damage << " damage!" << std::endl; }
                else { std::cout << enemy.name << " is shielded! No damage." << std::endl; enemy.shielded = false; }
            }
        }
        else {
            std::cout << "Choose an ally to shield: " << std::endl;
            int i = 1;
            for (auto& ally : allies) { std::cout << i << ". " << ally.name << std::endl; i++; }
            int choice; std::cin >> choice;
            auto it = allies.begin(); std::advance(it, choice - 1);
            it->shielded = true;
            std::cout << it->name << " is shielded!" << std::endl;
        }
    }
    else if (player.role == "Healer") {
        std::cout << "1. Heal Yourself," << std::endl;
        std::cout << "2. Damage, and Heal" << std::endl;
        std::cin >> Playermove;

        if (Playermove == 1) {
            player.heal(10);
            std::cout << player.name << " healed. HP: " << player.hp << "/" << player.maxhp << std::endl;
        }
        else {
            std::random_device rd; std::mt19937 gen(rd());
            std::uniform_int_distribution<> dist(0, enemies.size() - 1);
            int idx = dist(gen);
            auto it = enemies.begin(); 
            std::advance(it, idx);
            int damage = 8;
            if (!it->shielded) { it->hp -= damage; std::cout << it->name << " takes " << damage << " damage!" << std::endl; }
            else { std::cout << it->name << " is shielded! No damage." << std::endl; it->shielded = false; }

            for (auto& ally : allies) {
                ally.heal(damage / 2);
                std::cout << ally.name << " healed " << damage / 2 << " HP!" << std::endl;
            }
        }
    }
    else if (player.role == "Attacker") {
        std::cout << "1. Damage one Enemy," << std::endl;
        std::cout << "2. Damage all Enemies" << std::endl;
        std::cin >> Playermove;

        if (Playermove == 1) {
            std::random_device rd; std::mt19937 gen(rd());
            std::uniform_int_distribution<> dist(0, enemies.size() - 1);
            int idx = dist(gen);
            auto it = enemies.begin(); std::advance(it, idx);
            int damage = 15;
            if (!it->shielded) { it->hp -= damage; std::cout << it->name << " takes " << damage << " damage!" << std::endl; }
            else { std::cout << it->name << " is shielded! No damage." << std::endl; it->shielded = false; }
        }
        else {
            int damage = 5;
            for (auto& enemy : enemies) {
                if (!enemy.shielded) { enemy.hp -= damage; std::cout << enemy.name << " takes " << damage << " damage!" << std::endl; }
                else { std::cout << enemy.name << " is shielded! No damage." << std::endl; enemy.shielded = false; }
            }
        }
    }
}

struct Turn {
    Character* character;
    bool isPlayer;
};

void ShowBattleStatus(const std::list<PlayerCharacter>& players, const std::list<EnemyCharacter>& enemies) {
    std::cout << std::endl;
    std::cout << "=== Battle Status ===" << std::endl;

    std::cout << "Players: ";
    for (const auto& p : players) {
        int displayHP = std::max(p.hp, 0); 
        std::cout << p.name << " (HP: " << displayHP << "/" << p.maxhp << ") " 
            << "[AGI: " << p.agi << "] ";
        if (p.shielded) std::cout << "[SHIELDED]" << std::endl;
        else std::cout << std::endl;
    }
    std::cout << std::endl;

    std::cout << "Enemies: ";
    for (const auto& e : enemies) {
        int displayHP = std::max(e.hp, 0); 
        std::cout << e.name << " (HP: " << displayHP << "/" << e.maxhp << ") "
            << "[AGI: " << e.agi << "] ";
        if (e.shielded) std::cout << "[SHIELDED]" << std::endl;
        else std::cout << std::endl;
    }
    std::cout << std::endl;

    std::cout << "======================" << std::endl;
}

void Battlequeue(std::list<PlayerCharacter>& players, std::list<EnemyCharacter>& enemies) {
    std::list<Character*> participants;

    for (auto& p : players) participants.push_back(&p);
    for (auto& e : enemies) participants.push_back(&e);

   
    participants.sort([](Character* a, Character* b) {
        return a->agi > b->agi;
        });

   
    std::queue<Turn> turnOrder;
    for (auto* c : participants) {
        bool isPlayer = false;
        for (auto& p : players) {

            if (&p == c) {
                isPlayer = true;
                break;
            }
        }
        turnOrder.push({ c, isPlayer });
    }

    while (!players.empty() && !enemies.empty()) {

        if (turnOrder.empty()) break;

        Turn current = turnOrder.front();
        turnOrder.pop();

        if (current.character->hp <= 0) continue; 

        if (current.isPlayer) {
            auto* pc = static_cast<PlayerCharacter*>(current.character);
            PlayerTurn(*pc, players, enemies);
        }
        else {
            auto* ec = static_cast<EnemyCharacter*>(current.character);
            EnemyTurn(*ec, enemies, players);
        }
        ShowBattleStatus(players, enemies);
        
        if (current.character->hp > 0)
            turnOrder.push(current);

        
        players.remove_if([](PlayerCharacter& p) { return p.hp <= 0; });
        enemies.remove_if([](EnemyCharacter& e) { return e.hp <= 0; });
    }

    if (players.empty()) std::cout << "YOU LOST!" << std::endl;
    else std::cout << "YOU WON!" << std::endl;
}

int main() {
    std::list<PlayerCharacter> playerHeroes;
    std::list<PlayerCharacter> characters = loadCharacters();

    Menu();
    int ans;
    std::cin >> ans;
    std::cout << "----------------------" << std::endl;

    int picks = 0;
    while (picks < 3) {
        if (ans == 1) {
            std::cout << "1. Choose Your Heroes from list" << std::endl;
            std::cout << "2. Create a New Hero" << std::endl;
            int ch; std::cin >> ch;
            std::cout << "----------------------" << std::endl;

            if (ch == 1) {
                showCharacters(characters);
                std::cout << "Choose a Hero's number: ";
                int herochoice; std::cin >> herochoice;
                if (herochoice < 1 || herochoice >(int)characters.size()) {
                    std::cout << "Wrong choice, try again!" << std::endl;
                    continue;
                }

                auto it = characters.begin();
                std::advance(it, herochoice - 1);
                playerHeroes.push_back(*it);
                characters.erase(it);

                std::cout << "You Chose: " << playerHeroes.back().name << std::endl;
                std::cout << "----------------------" << std::endl;

                picks++;
                std::cout << "All Your Heroes:" << std::endl;
                showCharacters(playerHeroes);
                std::cout << "----------------------" << std::endl;
            }
            else if (ch == 2) {
                PlayerCharacter newHero = createHero();
                characters.push_back(newHero);
                std::cout << "Your Hero " << newHero.name << " added to list" << std::endl;
                continue;
            }
        }
        else { return 0; }
    }

    if (picks == 3) {
        std::list<EnemyCharacter> allEnemies = loadEnemies();
        std::list<EnemyCharacter> enemies = getRandomEnemies(allEnemies);
        Battlequeue(playerHeroes, enemies);

    }

    return 0;
}
