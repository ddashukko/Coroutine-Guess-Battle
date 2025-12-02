//variant: 11
#include <iostream>
#include <coroutine>
#include <random>
#include <chrono>

struct GuessGenerator {
    struct promise_type {
        int current_guess;

        GuessGenerator get_return_object() {
            return GuessGenerator{ std::coroutine_handle<promise_type>::from_promise(*this) };
        }

        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }

        std::suspend_always yield_value(int value) {
            current_guess = value;
            return {};
        }

        void return_void() {}
        void unhandled_exception() { std::terminate(); }
    };

    std::coroutine_handle<promise_type> handle;

    GuessGenerator(std::coroutine_handle<promise_type> h) : handle(h) {}
    ~GuessGenerator() {
        if (handle) handle.destroy();
    }

    int get_guess() const {
        return handle.promise().current_guess;
    }

    bool done() const {
        return handle.done();
    }

    void resume() {
        if (!handle.done()) handle.resume();
    }
};

GuessGenerator player_guesser(int id) {
    std::random_device rd;
    std::mt19937 gen(rd() + id);
    std::uniform_int_distribution<> distrib(1, 100);

    while (true) {
        int guess = distrib(gen);
        co_yield guess;
    }
}

int main() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(1, 100);

    int secret_number = distrib(gen);

    std::cout << "=== GAME START ===" << std::endl;
    std::cout << "Dealer has chosen a secret number [1-100]." << std::endl;
    std::cout << "-----------------------------------------" << std::endl;

    auto player1 = player_guesser(1);
    auto player2 = player_guesser(2);

    int turn = 0;
    bool game_over = false;

    while (!game_over) {
        turn++;

        // --- Player 1 Turn ---
        player1.resume();
        int p1_guess = player1.get_guess();
        std::cout << "[Turn " << turn << "] Player 1 guesses: " << p1_guess;

        if (p1_guess == secret_number) {
            std::cout << " -> CORRECT! Player 1 WINS!" << std::endl;
            game_over = true;
            break;
        }
        else if (p1_guess < secret_number) {
            std::cout << " (Too small)" << std::endl;
        }
        else {
            std::cout << " (Too big)" << std::endl;
        }

        // --- Player 2 Turn ---
        player2.resume();
        int p2_guess = player2.get_guess();
        std::cout << "[Turn " << turn << "] Player 2 guesses: " << p2_guess;

        if (p2_guess == secret_number) {
            std::cout << " -> CORRECT! Player 2 WINS!" << std::endl;
            game_over = true;
            break;
        }
        else if (p2_guess < secret_number) {
            std::cout << " (Too small)" << std::endl;
        }
        else {
            std::cout << " (Too big)" << std::endl;
        }
    }

    std::cout << "-----------------------------------------" << std::endl;
    std::cout << "The secret number was: " << secret_number << std::endl;
    std::cout << "=== GAME OVER ===" << std::endl;

    return 0;
}