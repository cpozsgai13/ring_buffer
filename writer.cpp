#include <boost/lockfree/spsc_queue.hpp> // ring buffer

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/string.hpp>

#include <iostream>
#include <string>
using namespace std;

namespace bip = boost::interprocess;
namespace shm
{
    typedef bip::allocator<char, bip::managed_shared_memory::segment_manager> char_alloc;
    typedef bip::basic_string<char, std::char_traits<char>, char_alloc >      shared_string;

    typedef boost::lockfree::spsc_queue<
        shared_string, 
        boost::lockfree::capacity<200> 
    > ring_buffer;
}

#include <unistd.h>

int main()
{
    // create segment and corresponding allocator
    bip::managed_shared_memory segment(bip::open_or_create, "WHATISUP", 32*2048);
    shm::char_alloc char_alloc(segment.get_segment_manager());

    // Ringbuffer fully constructed in shared memory. The element strings are
    // also allocated from the same shared memory segment. This vector can be
    // safely accessed from other processes.
    shm::ring_buffer *queue = segment.find_or_construct<shm::ring_buffer>("queue")();

    cout << "Enter text to write to shared memory segment or Q to quit" << endl;
    string input;
    cin >> input;
    while(input != "Q") {
        queue->push(shm::shared_string(input, char_alloc));
        cin >> input;
    }
}
