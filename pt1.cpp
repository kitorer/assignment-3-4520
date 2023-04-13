#include <thread>
#include <mutex>
#include <condition_variable>
#include <unordered_set>
#include <vector>
#include <iostream>
#include <semaphore.h>
#include <random>

const int NUM_SERVANTS = 4;
const int NUM_GIFTS = 500000;

struct Gift {
    int tag;
    Gift* next;

    Gift(int tag) : tag(tag), next(nullptr) {}
};

class ConcurrentLinkedList {
public:
    ConcurrentLinkedList() : head(nullptr), tail(nullptr), nextTag(0) {
    sem_init(&semaphore, 0, 0);    
    }


    void addGift(Gift* gift) {
        std::unique_lock<std::mutex> lock(m);
        if (giftSet.find(gift->tag) != giftSet.end()) {
            std::cout << "Gift with tag " << gift->tag << " is already in the list." << std::endl;
            return;
        }
        if (!tail) {
            head = gift;
            tail = gift;
        } else {
            tail->next = gift;
            tail = gift;
        }
        giftSet.insert(gift->tag);
        std::cout << "Added gift with tag " << gift->tag << " to the list." << std::endl;
        sem_post(&semaphore);
    }

    void removeGift(int tag) {
        std::unique_lock<std::mutex> lock(m);
        cv.wait(lock, [this]() { return head != nullptr; }); // wait until there is a gift to remove
        Gift* prev = nullptr;
        for (Gift* curr = head; curr; prev = curr, curr = curr->next) {
            if (curr->tag == tag) {
                if (prev) {
                    prev->next = curr->next;
                } else {
                    head = curr->next;
                }
                if (!curr->next) {
                    tail = prev;
                }
                delete curr;
                giftSet.erase(tag);
                std::cout << "Removed gift with tag " << tag << std::endl;
                return;
            }
        }
        std::cout << "Gift with tag " << tag << " not found in the list." << std::endl;
    }

    bool containsGift(int tag) {
        std::unique_lock<std::mutex> lock(m);
        bool contains = giftSet.find(tag) != giftSet.end();
        std::cout << "Checked if gift with tag " << tag << " is in the list. Result: " << contains << std::endl;
        return contains;
    }

void thankYouCards(int servantId) {
        while (true) {
            sem_wait(&semaphore);
            std::unique_lock<std::mutex> lock(m);
            int action = rand() % 3;
            if (action == 0) {
                Gift* gift = new Gift(++nextTag);
                addGift(gift);
            } else if (action == 1) {
                if (!giftSet.empty()) {
                    int tag = *giftSet.begin();
                    removeGift(tag);
                    std::cout << "Servant " << servantId << " removed gift with tag " << tag << std::endl;
                    numCards[servantId]++;
                    lock.unlock(); // unlock the mutex before printing the message
                    std::cout << "Servant " << servantId << " wrote a thank-you card." << std::endl;
                }
            } else {
                if (nextTag > 0) { // don't check for a gift with tag 0
                    int tag = rand() % nextTag + 1;
                    bool contains = containsGift(tag);
                    std::cout << "Servant " << servantId << " checked if gift with tag " << tag << " is in the list. Result: " << contains << std::endl;
                }
            }
            if (giftSet.empty()) {
                break;
            }
        }
    }
    
    int* getNumCards() {
        return numCards;
    }
    int nextTag = 0;
    Gift* head;
    Gift* tail;
    std::unordered_set<int> giftSet;
    std::mutex m;
    std::condition_variable cv;
    sem_t semaphore;
    int numCards[NUM_SERVANTS] = {0};
    int numChecks[NUM_SERVANTS] = {0};
};

int main() {
    ConcurrentLinkedList list;
    std::vector<std::thread> threads;

    for (int i = 0; i < NUM_SERVANTS; i++) {
        threads.emplace_back(&ConcurrentLinkedList::thankYouCards, &list, i);
    }

    for (int i = 1; i <= NUM_GIFTS; i++) {
        list.addGift(new Gift(i));
    }

    for (auto& thread : threads) {
        thread.join();
    }
    int* numCards = list.getNumCards();
    for (int i = 0; i < NUM_SERVANTS; i++) {
        std::cout << "Servant " << i << " wrote " << numCards[i] << " thank you cards." << std::endl;
    }

    return 0;
}
