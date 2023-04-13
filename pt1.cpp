#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <unordered_set>
#include <random>

using namespace std;

const int kNumServants = 4;
const int kNumPresents = 500000;

// Class for a present
class Present {
public:
    Present(int tag) : tag_(tag), next_(nullptr) {}

    int tag_;
    Present* next_;
};

// Class for a linked-list of presents
class PresentList {
public:
    PresentList() : head_(nullptr), tail_(nullptr) {}

    void AddPresent(Present* present) {
        // Empty list
        if (!head_) {
            head_ = present;
            tail_ = present;
        }
        // Add to the beginning
        else if (head_->tag_ > present->tag_) {
            present->next_ = head_;
            head_ = present;
        }
        // Add to the end
        else if (tail_->tag_ < present->tag_) {
            tail_->next_ = present;
            tail_ = present;
        }
        // Add in the middle
        else {
            Present* prev = head_;
            while (prev->next_->tag_ < present->tag_) {
                prev = prev->next_;
            }
            present->next_ = prev->next_;
            prev->next_ = present;
        }
    }

    void RemovePresent(int tag) {
        Present* curr = head_;
        Present* prev = nullptr;

        while (curr && curr->tag_ != tag) {
            prev = curr;
            curr = curr->next_;
        }

        // If not found
        if (!curr) {
            return;
        }

        // Remove from the beginning
        if (!prev) {
            head_ = curr->next_;
            // If the list is now empty
            if (!head_) {
                tail_ = nullptr;
            }
        }
        // Remove from the end
        else if (curr == tail_) {
            tail_ = prev;
            prev->next_ = nullptr;
        }
        // Remove from the middle
        else {
            prev->next_ = curr->next_;
        }

        delete curr;
    }

    bool HasPresent(int tag) {
        Present* curr = head_;
        while (curr && curr->tag_ < tag) {
            curr = curr->next_;
        }
        return curr && curr->tag_ == tag;
    }

    Present* head_;
    Present* tail_;
};

// Shared data between the threads
struct SharedData {
    vector<int> presents;
    unordered_set<int> present_tags;
    PresentList present_list;
    mutex presents_mutex;
    mutex present_tags_mutex;
    mutex present_list_mutex;
    condition_variable presents_cv;
    condition_variable present_tags_cv;
    condition_variable present_list_cv;
    int num_thank_you_notes = 0;
    mutex num_thank_you_notes_mutex;
};

// Function for the thread that adds presents to the linked-list
void AddPresentsThread(SharedData* shared_data, int servant_id) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> distrib(1, kNumPresents);

    while (true) {
        // Wait until there is at least 1 present in the bag
        unique_lock<mutex> lock(shared_data->presents_mutex);
        shared_data->presents_cv.wait(lock, [&shared_data]{ return !shared_data->presents.empty(); });

        // Get the present from the bag
        int present = shared_data->presents.back();
        shared_data->presents.pop_back();
        
