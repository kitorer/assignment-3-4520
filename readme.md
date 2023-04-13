# problem 1
`g++ -O2 -std=c++2a pt2.cpp -pthread` and `./a.out`
## run-down
Minotaur has a birthday, he received 500k presents from his guests he has 4 servants(threads) write thank you cards for everyone.Each servant could add a present to the chain, remove a present and write a card, or check whether a present with a certain tag number was already in the chain. The servants alternated between adding presents to the chain and writing cards until all the cards were written and all presents were in the chain. However, at the end of the day, they realized that they had more presents than "Thank you" cards.

what could have gone wrong was that they might have added some extra thank you notes to some nodes, proper communication on which presents are being worked on could prevent this.
 ## Methods
I am aware that lazy locking could have been the better solution for this problem however I found that out too late.
I used mutexes and semaphores top ensure thread safety, and automic variables to ensure that the shared variables are updated automatically. The semaphores signal that a new gift has been added to the list.

# problem 2

## run-down

This program simulates temperatures with 8 sensors, so each sensor is given a thread to work with and they pile their data into shared memory. we need 5 highest, lowest and 10 minute snapshot of when the largest temp was recorded.
## explenation

theres 8 sensors, and 8 threads. Each sensor works with a thread. Shared memory results in faster reads. 
But there is a catch, because we need to get the 5 highest and lowest temps it takes computational time to compute that, however our input size does not grow, it stays constant since we will always have 60*8=720 readings to sort through every hour.

## Methods

We use a mutex to protect memory in the shared memory from being overwritten, and there is a seperate thread to reading all of the data so that only completed temperature readings are happening. This is better than using one of the temperature threads to compile the data becuase that way there can be no delay in readings as a result of computational work and any delay in readings does not change the compiled results.
