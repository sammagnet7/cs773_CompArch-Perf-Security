#include "cache.h"

uint32_t CACHE::lru_victim_WAY_PARTITION(uint32_t cpu, uint64_t instr_id, uint32_t set, const BLOCK *current_set, uint64_t ip, uint64_t full_addr, uint32_t type)
{
    uint32_t way = 0;
    uint32_t st_way = 0, end_way = 0;

    if (cpu == 0)
    {
        st_way = 0;
        end_way = NUM_WAY / 2;
    }
    else if (cpu == 1)
    {
        st_way = NUM_WAY / 2;
        end_way = NUM_WAY;
    }
    // fill invalid line first
    for (way = st_way; way < end_way; way++)
    {
        if (block[set][way].valid == false)
        {

            DP(if (warmup_complete[cpu]) {
            cout << "[" << NAME << "] " << __func__ << " instr_id: " << instr_id << " invalid set: " << set << " way: " << way;
            cout << hex << " address: " << (full_addr>>LOG2_BLOCK_SIZE) << " victim address: " << block[set][way].address << " data: " << block[set][way].data;
            cout << dec << " lru: " << block[set][way].lru << endl; });

            break;
        }
    }

    // LRU victim
    if (way == end_way)
    {
        for (way = st_way; way < end_way; way++)
        {
            if (block[set][way].lru == NUM_WAY / 2 - 1)
            {

                DP(if (warmup_complete[cpu]) {
                cout << "[" << NAME << "] " << __func__ << " instr_id: " << instr_id << " replace set: " << set << " way: " << way;
                cout << hex << " address: " << (full_addr>>LOG2_BLOCK_SIZE) << " victim address: " << block[set][way].address << " data: " << block[set][way].data;
                cout << dec << " lru: " << block[set][way].lru << endl; });

                break;
            }
        }
    }

    if (way == end_way)
    {
        cerr << "[" << NAME << "] " << __func__ << " no victim! set: " << set << endl;
        assert(0);
    }

    return way;
}

void CACHE::lru_update_WAY_PARTITION(uint32_t set, uint32_t way, uint32_t cpu)
{
    uint32_t st_way = 0, end_way = 0;

    if (cpu == 0)
    {
        st_way = 0;
        end_way = NUM_WAY / 2;
    }
    else if (cpu == 1)
    {
        st_way = NUM_WAY / 2;
        end_way = NUM_WAY;
    }

    // update lru replacement state
    for (uint32_t i = st_way; i < end_way; i++)
    {
        if (block[set][i].lru < block[set][way].lru)
        {
            block[set][i].lru++;
        }
    }
    block[set][way].lru = 0; // promote to the MRU position
}

// initialize replacement state
void CACHE::llc_initialize_replacement()
{
    cout << NAME << " has LRU replacement policy" << endl;
}

// find replacement victim
uint32_t CACHE::llc_find_victim(uint32_t cpu, uint64_t instr_id, uint32_t set, const BLOCK *current_set, uint64_t ip, uint64_t full_addr, uint32_t type)
{
    // baseline LRU
    return lru_victim_WAY_PARTITION(cpu, instr_id, set, current_set, ip, full_addr, type);
}

// called on every cache hit and cache fill
void CACHE::llc_update_replacement_state(uint32_t cpu, uint32_t set, uint32_t way, uint64_t full_addr, uint64_t ip, uint64_t victim_addr, uint32_t type, uint8_t hit)
{

    if ((type == WRITEBACK) && ip)
        assert(0);

    // uncomment this line to see the LLC accesses
    // cout << "CPU: " << cpu << "  LLC " << setw(9) << TYPE_NAME << " set: " << setw(5) << set << " way: " << setw(2) << way;
    // cout << hex << " paddr: " << setw(12) << paddr << " ip: " << setw(8) << ip << " victim_addr: " << victim_addr << dec << endl;

    // baseline LRU
    if (hit && (type == WRITEBACK)) // writeback hit does not update LRU state
        return;

    if ((cpu == 0 && way >= NUM_WAY / 2) || (cpu == 1 && way < NUM_WAY / 2))
    {
        cerr << "[" << NAME << "] " << __func__ << " CPU PARTITION WAY MISMATCH! cpu: " << cpu << " way: " << way << endl;

        assert(0);
    }

    return lru_update_WAY_PARTITION(set, way, cpu);
}

void CACHE::llc_replacement_final_stats()
{
}
