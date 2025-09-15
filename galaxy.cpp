// Project identifier: AD48FB4835AF347EB0CA8009E24C3B13F8519882

#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <getopt.h>

#include "P2random.h"

// OPTIONS

void printHelp(char *command) {
    std::cout << "Usage:" << command << "{options}\n\n";
    std::cout << "Options:              Prints:\n";
    std::cout << "-h, --help            help message\n";
    std::cout << "-v, --verbose         battle summary\n";
    std::cout << "-m, --median          median troops lost per planet after time change, if troops were lost\n";
    std::cout << "-g, --general-eval    troop deployment and survival under each general\n";
    std::cout << "-w, --watcher         most interesting battle possible per planet, if one exists.";
}

bool verbose_mode = false;
bool median_mode = false;
bool general_eval_mode = false;
bool watcher_mode = false;

void getOptions(int argc, char **argv) {
    struct option long_options[] = {
            {"help", no_argument, nullptr, 'h'},
            {"verbose", no_argument, nullptr, 'v'},
            {"median", no_argument, nullptr, 'm'},
            {"general-eval", no_argument, nullptr, 'g'},
            {"watcher", no_argument, nullptr, 'w'},
            { nullptr, 0, nullptr, '\0' }
        };

    int choice = 0;
    int option_index = 0;


    while ((choice = getopt_long(argc, argv, "hvmgw", long_options, &option_index)) != -1) {
        switch(choice) {
            
            case 'h':
                printHelp(* argv);
                exit(0);
            case 'v':
                verbose_mode = true;
                break;
            case 'm':
                median_mode = true;
                break;
            case 'g':
                general_eval_mode = true;
                break;
            case 'w':
                watcher_mode = true;
                break;
            default:
                std::cerr << "Error: invalid option\n" << std::flush;
                exit(1);
        }
    }

}







// MODULAR DATA TYPES: Battalion, Planet, General, SimulationInput

struct Battalion {
    int timestamp;
    uint32_t generalID;
    uint32_t planetID;
    bool isJedi;
    int forceSensitivity;
    int numTroops;
    int arrivalOrder; // To break ties in priority queues
};

// // Comparators
struct JediComparator {
    bool operator() (Battalion const & a, Battalion const &b) const {
        if(a.forceSensitivity > b.forceSensitivity) {
            return true;
        } 
        else if ((a.forceSensitivity == b.forceSensitivity) && (a.arrivalOrder > b.arrivalOrder)) {
            return true;
        }
        return false;
    }
};
struct SithComparator {
    bool operator() (Battalion const &a, Battalion const &b) const {
        if((a.forceSensitivity < b.forceSensitivity) && b.numTroops !=0) {
            return true;
        } 
        else if ((a.forceSensitivity == b.forceSensitivity) && (a.arrivalOrder > b.arrivalOrder)) {
            return true;
        }
        return false;
    }
};

class Planet {
    public:
    std::priority_queue<Battalion, std::vector<Battalion>, JediComparator> jediPQ;
    std::priority_queue<Battalion, std::vector<Battalion>, SithComparator> sithPQ;

    // std::vector<int> troopsLostHistory; // For median tracking

    std::priority_queue<int> low; // Max-heap
    std::priority_queue<int, std::vector<int>, std::greater<int>> high; // Min-heap

    std::vector<Battalion> allJediDeployments; // For movie watcher mode
    std::vector<Battalion> allSithDeployments; // For movie watcher mode

    // MovieWatcherData
    struct {
        Battalion min_attack_jedi;
        Battalion min_ambush_jedi;
        Battalion max_attack_sith;
        Battalion max_ambush_sith;
        bool has_ambush_jedi = false;
        bool has_ambush_sith = false;
        bool has_attack_jedi = false;
        bool has_attack_sith = false;
        Battalion maybe_attack_jedi;
        Battalion maybe_ambush_sith;
    } movie_data;
};

struct General {
    int num_jedi_deployed = 0;
    int num_sith_deployed = 0;
    int jedi_lost = 0;
    int sith_lost = 0;
};

struct SimulationInput {
    uint32_t NUM_GENERALS;
    uint32_t NUM_PLANETS;
    std::vector<General> generals;
    std::vector<Planet> planets;
    
    SimulationInput(uint32_t ng, uint32_t np) 
        : NUM_GENERALS(ng), NUM_PLANETS(np),
          generals(ng), // Initialize vector with size ng
          planets(np)   // Initialize vector with size np
    {};
};



// General-eval Output
void printGeneralEvalOutput(std::vector<General> const & generals) {
        std::cout << "---General Evaluation---\n";
        for(uint32_t i=0; i < generals.size(); ++i) {
            std::cout << "General " << i << " deployed " << generals[i].num_jedi_deployed << " Jedi troops and " <<
            generals[i].num_sith_deployed << " Sith troops, and "<< ((generals[i].num_jedi_deployed + generals[i].num_sith_deployed) - (generals[i].jedi_lost + generals[i].sith_lost)) << '/' << (generals[i].num_jedi_deployed + generals[i].num_sith_deployed) <<
            " troops survived.\n";
        }
}

void printMovieWatcherOutput(SimulationInput const & simput) {
    std::cout << "---Movie Watcher---\n";
    for(uint32_t i = 0; i < simput.planets.size(); ++i) {

        // AMBUSH
        if (simput.planets[i].movie_data.has_ambush_jedi && simput.planets[i].movie_data.has_ambush_sith &&
        (simput.planets[i].movie_data.max_ambush_sith.forceSensitivity - simput.planets[i].movie_data.min_ambush_jedi.forceSensitivity >= 0)) {
            std::cout << "On planet " << i << ", a movie watcher would enjoy an ambush with "
                    << "Sith at time " << simput.planets[i].movie_data.max_ambush_sith.timestamp 
                    << " and Jedi at time " << simput.planets[i].movie_data.min_ambush_jedi.timestamp
                    << " with a force difference of " << (simput.planets[i].movie_data.max_ambush_sith.forceSensitivity - simput.planets[i].movie_data.min_ambush_jedi.forceSensitivity) << ".\n";
        }
        else {
            std::cout << "On planet " << i << ", a movie watcher would not see an interesting ambush.\n";
        }
        // ATTACK
        if (simput.planets[i].movie_data.has_attack_jedi && simput.planets[i].movie_data.has_attack_sith && 
        (simput.planets[i].movie_data.max_attack_sith.forceSensitivity - simput.planets[i].movie_data.min_attack_jedi.forceSensitivity >= 0)) {
            std::cout << "On planet " << i << ", a movie watcher would enjoy an attack with "
                    << "Jedi at time " << simput.planets[i].movie_data.min_attack_jedi.timestamp
                    << " and Sith at time " << simput.planets[i].movie_data.max_attack_sith.timestamp
                    << " with a force difference of " << (simput.planets[i].movie_data.max_attack_sith.forceSensitivity - simput.planets[i].movie_data.min_attack_jedi.forceSensitivity) << ".\n";
        }
        else {
            std::cout << "On planet " << i << ", a movie watcher would not see an interesting attack.\n";
        }
    }

}



void addTroopsLost(Planet &planet, int troopsLost) {
    if (planet.low.empty() || troopsLost <= planet.low.top()) {
        planet.low.push(troopsLost);
    } else {
        planet.high.push(troopsLost);
    }

    // Rebalance the heaps to ensure size property is maintained
    if (planet.low.size() > planet.high.size() + 1) {
        planet.high.push(planet.low.top());
        planet.low.pop();
    } else if (planet.high.size() > planet.low.size()) {
        planet.low.push(planet.high.top());
        planet.high.pop();
    }
}

void printMedianOut(std::vector<Planet> &planets, int timestamp) {
    for(uint32_t i = 0; i < planets.size(); i++)
        // if low.size() > high.size(), odd number of troopsLost values. No division by 2. Median = low.top() 
        if (planets[i].low.size() > planets[i].high.size()) {
            std::cout << "Median troops lost on planet " << i << " at time " << timestamp << " is " << planets[i].low.top() << ".\n";
        }
        // if high.size() > low.size(), odd number of troopsLost values. No division by 2. Median = high.top()
        else if (planets[i].high.size() > planets[i].low.size()) {
            std::cout << "Median troops lost on planet " << i << " at time " << timestamp << " is " << planets[i].high.top() << ".\n";
        }
        // if low.size() != 0, high.size() !=0 too (since we add troopsLost to low first if low is empty). if high.size() == low.size(), two middle troopsLost values. Divide by 2
        else if ((planets[i].low.size() != 0) && (planets[i].high.size() == planets[i].low.size())){
            std::cout << "Median troops lost on planet " << i << " at time " << timestamp << " is " << ((planets[i].low.top() + planets[i].high.top()) / 2) << ".\n";
        }
}




void battle(Planet& planet, std::vector<General>& generals) {
    Battalion jedi = planet.jediPQ.top();
    Battalion sith = planet.sithPQ.top();
    planet.jediPQ.pop();
    planet.sithPQ.pop();
    
    int troopsLost = std::min(jedi.numTroops, sith.numTroops);
    
    // Track losses instead of survivors
    generals[jedi.generalID].jedi_lost += troopsLost;
    generals[sith.generalID].sith_lost += troopsLost;
    
    // Update and reinsert
    jedi.numTroops -= troopsLost;
    sith.numTroops -= troopsLost;
    
    if (jedi.numTroops > 0) planet.jediPQ.push(jedi);
    if (sith.numTroops > 0) planet.sithPQ.push(sith);
    
    // planet.troopsLostHistory.push_back(troopsLost * 2);
    addTroopsLost(planet, (troopsLost * 2));
    
    // VERBOSE OUTPUT
    if(verbose_mode == true) {
        std::cout << "General " << sith.generalID << "'s battalion attacked General " << jedi.generalID << "'s battalion on planet " << jedi.planetID << ". ";
        std::cout << (troopsLost * 2) << " troops were lost.\n";
    }
}




// Read Deployment List format
void readDL(SimulationInput& simput, uint32_t num_generals, uint32_t num_planets, std::istream &input) {
    Battalion battalion;
    std::string side;
    char discard;
    battalion.arrivalOrder = 0;
    int previous_timestamp = 0;
    int current_timestamp = 0;
    int total_battles = 0;

    while (input >> battalion.timestamp >> side >> discard >>
           battalion.generalID >> discard >> battalion.planetID >> discard >>
           battalion.forceSensitivity >> discard >> battalion.numTroops) {

        // Validate input
        if (battalion.generalID >= num_generals || battalion.planetID >= num_planets ||
            battalion.forceSensitivity <= 0 || battalion.numTroops <= 0) {
            std::cerr << "Invalid deployment data\n";
            exit(1);
        }

        // Check non-decreasing timestamp
        if (battalion.timestamp < previous_timestamp) {
            std::cerr << "Invalid decreasing timestamp\n";
            exit(1);
        }
        previous_timestamp = battalion.timestamp;

        battalion.arrivalOrder++;
        Planet& current = simput.planets[battalion.planetID];

        if (side == "JEDI") {
            battalion.isJedi = true;
            current.allJediDeployments.push_back(battalion);
            current.jediPQ.push(battalion);
            simput.generals[battalion.generalID].num_jedi_deployed += battalion.numTroops;

            if (watcher_mode) {
                // AMBUSH check (Sith has arrived already)
                if (current.movie_data.has_ambush_sith) {
                    Battalion sith = current.movie_data.maybe_ambush_sith;
                    if (sith.timestamp <= battalion.timestamp &&
                        sith.forceSensitivity >= battalion.forceSensitivity) {
                        int diff = sith.forceSensitivity - battalion.forceSensitivity;
                        bool should_replace = false;

                        if (!current.movie_data.has_ambush_jedi) {
                            should_replace = true;
                        } else {
                            int curr_diff = current.movie_data.max_ambush_sith.forceSensitivity - 
                                            current.movie_data.min_ambush_jedi.forceSensitivity;
                            if (diff > curr_diff ||
                               (diff == curr_diff && sith.timestamp < current.movie_data.max_ambush_sith.timestamp)) {
                                should_replace = true;
                            }
                        }

                        if (should_replace) {
                            current.movie_data.max_ambush_sith = sith;
                            current.movie_data.min_ambush_jedi = battalion;
                            current.movie_data.has_ambush_jedi = true;
                        }
                    }
                }

                // Track lowest-force Jedi for future attack pairing
                if (!current.movie_data.has_attack_jedi || 
                    battalion.forceSensitivity < current.movie_data.maybe_attack_jedi.forceSensitivity) {
                    current.movie_data.maybe_attack_jedi = battalion;
                    current.movie_data.has_attack_jedi = true;
                }
            }

        } else {  // SITH
            battalion.isJedi = false;
            current.allSithDeployments.push_back(battalion);
            current.sithPQ.push(battalion);
            simput.generals[battalion.generalID].num_sith_deployed += battalion.numTroops;

            if (watcher_mode) {
                // Save strongest Sith for future ambush pairing
                if (!current.movie_data.has_ambush_sith || 
                    battalion.forceSensitivity > current.movie_data.maybe_ambush_sith.forceSensitivity) {
                    current.movie_data.maybe_ambush_sith = battalion;
                    current.movie_data.has_ambush_sith = true;
                }

                // ATTACK check (Jedi has arrived before)
                if (current.movie_data.has_attack_jedi) {
                    Battalion jedi = current.movie_data.maybe_attack_jedi;
                    if (jedi.timestamp <= battalion.timestamp &&
                        battalion.forceSensitivity >= jedi.forceSensitivity) {
                        int diff = battalion.forceSensitivity - jedi.forceSensitivity;
                        bool should_replace = false;

                        if (!current.movie_data.has_attack_sith) {
                            should_replace = true;
                        } else {
                            int curr_diff = current.movie_data.max_attack_sith.forceSensitivity - 
                                            current.movie_data.min_attack_jedi.forceSensitivity;
                            if (diff > curr_diff ||
                               (diff == curr_diff && jedi.timestamp < current.movie_data.min_attack_jedi.timestamp)) {
                                should_replace = true;
                            }
                        }

                        if (should_replace) {
                            current.movie_data.max_attack_sith = battalion;
                            current.movie_data.min_attack_jedi = jedi;
                            current.movie_data.has_attack_sith = true;
                        }
                    }
                }
            }
        }

        // Median output check
        if (battalion.timestamp != current_timestamp) {
            if (median_mode) {
                printMedianOut(simput.planets, current_timestamp);
            }
            current_timestamp = battalion.timestamp;
        }

        // Battle triggering (verbose-safe)
        while (!current.jediPQ.empty() && !current.sithPQ.empty() &&
               current.sithPQ.top().forceSensitivity >= current.jediPQ.top().forceSensitivity) {
            battle(current, simput.generals);
            total_battles++;
        }
    }

    // Final median output
    if (median_mode) {
        printMedianOut(simput.planets, current_timestamp);
    }

    // End-of-day summary
    std::cout << "---End of Day---\n";
    std::cout << "Battles: " << total_battles << "\n";

    // Optional summary modes
    if (general_eval_mode) {
        printGeneralEvalOutput(simput.generals);
    }

    if (watcher_mode) {
        printMovieWatcherOutput(simput);
    }
}







// General Read Func
void readInput() {
    std::cout << "Deploying troops...\n";
// Header Info
    uint32_t num_generals;
    uint32_t num_planets;
    std::string junk;
    std::string mode;

// Reading Header
    getline(std::cin, junk);
    std::cin >> junk;
    std::cin >> mode;
    std::cin >> junk;
    std::cin >> num_generals;
    std::cin >> junk;
    std::cin >> num_planets;
    SimulationInput simInput(num_generals, num_planets);

// Reading Deployment List Input
    if(mode == "DL") {
        readDL(simInput, num_generals, num_planets, std::cin);
    }
// Reading Pseudo-Random Input
    else {
        std::stringstream ss;
        uint32_t seed;
        uint32_t num_deployments;
        uint32_t arrival_rate;
        std::string junk;
        std::cin >> junk >> seed >> junk >> num_deployments >> junk >> arrival_rate;
        
        P2random::PR_init(ss, seed, num_generals, num_planets, num_deployments, arrival_rate);
        readDL(simInput, num_generals, num_planets, ss);
    }
}





int main(int argc, char * argv[]) {
    std::ios_base::sync_with_stdio(false);
    getOptions(argc, argv);
    readInput();

}