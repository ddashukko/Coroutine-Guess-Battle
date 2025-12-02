//variant: 11
#include <iostream>
#include <thread>
#include <random>
#include <string>
#include <atomic>   
#include <chrono> 
#include <syncstream>

using namespace std;  
 
int random_number(int min, int max) { 
    static random_device rd;
    static mt19937 mt(rd());
    uniform_int_distribution<int> dist(min, max);
    return dist(mt);
}

void player(const string& name, int secret, atomic<bool>& finished) {
    while (!finished.load()) {
        int guess = random_number(1, 100);
        {
            osyncstream out(cout);
            out << name << " guesses: " << guess << endl;
        }
        if (guess == secret) {
            bool expected = false;
            if (finished.compare_exchange_strong(expected, true)) {
                osyncstream out(cout);
                out << name << " WINS!" << endl;
            }
            return;
        }
        this_thread::sleep_for(chrono::milliseconds(200));
    }
}

int main() {
    int secret = random_number(1, 100);
    {
        osyncstream out(cout);
        out << "Secret number chosen." << endl;
    }

    atomic<bool> finished(false);

    thread t1(player, "Player1", secret, ref(finished));
    thread t2(player, "Player2", secret, ref(finished));

    t1.join();
    t2.join();

    {
        osyncstream out(cout);
        out << "Game over!" << endl;
    }

    return 0;
}
