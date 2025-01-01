// Seth Pavlicek

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <climits>
#include <map>
#include <set>
#include <stdexcept>

using namespace std;

const string STATION_HEADER = "[Stations]";
const string AVAILABILITY_REPORT_HEADER = "[Charger Availability Reports]";


/**
 * =============
 * errorStation
 * =============
 * custom error class
*/
class errorStation : public runtime_error {

    public:
        errorStation(const string& errorStationMessage = "") : runtime_error(errorStationMessage) {

        }
};

/**
 * =============
 * Station
 * =============
 * - manages the uptime for a single station
 * - calculates the uptime for a station
 * 
*/
class Station {
    private:
        int stationID;

        long minTime;
        long maxTime;

        bool empty;

        vector<pair<long, long>*> timeSequences;

        // check if the sequence at index j overlaps with other sequences after it
        void nextCheck(int j) {
            for (int k = j + 1; k < timeSequences.size(); k++) {
                if (timeSequences[j]->second >= timeSequences[k]->second) {
                    timeSequences.erase(timeSequences.begin() + k);
                }
                if (timeSequences[j]->second >= timeSequences[k]->first) {
                    timeSequences[j]->second = timeSequences[k]->second;
                    timeSequences.erase(timeSequences.begin() + k);
                    break;
                }
            }
        }

        // check if the sequence at index j overlaps with other sequences before it
        void prevCheck(int j) {
            for (int k = j - 1; k > -1; k--) {
                if (timeSequences[j]->first <= timeSequences[k]->first) {
                    timeSequences.erase(timeSequences.begin() + k);
                }
                if (timeSequences[j]->first < timeSequences[k]->second) {
                    timeSequences[j]->first = timeSequences[k]->first;
                    timeSequences.erase(timeSequences.begin() + k);
                    break;
                }
            }
        }

        // check if a time sequence is within the time sequence at index j
        int underlapCheck(int j, pair<long,long> *timeSeq) {
            if (timeSeq->first >= timeSequences[j]->first && timeSeq->second <= timeSequences[j]->second) {
                // do not need to insert this sequence as it is redundant
                return 1;
            } 
            return 0;
        }

        // check if a time sequence completely overlaps the one at index j in timeSequences
        int supercedeCheck(int j, pair<long,long> *timeSeq) {
            if (timeSeq->first < timeSequences[j]->first && timeSeq->second > timeSequences[j]->second) {
                timeSequences[j]->second = timeSeq->second;
                timeSequences[j]->first = timeSeq->first;

                // check if the sequence keeps overlapping with the next sequences
                nextCheck(j);
                // check if the sequence keeps overlapping with the previous sequences
                prevCheck(j);
                return 1;
            }
            return 0;
        }

        // check if a time sequence overlaps the right side of the one at index j in timeSequences
        int overlapGreaterCheck(int j, pair<long,long> *timeSeq) {
            if (timeSeq->first <= timeSequences[j]->second && timeSeq->second > timeSequences[j]->second) {
                // combine the sequences due to overlap
                timeSequences[j]->second = timeSeq->second;
                
                // check if the new combined sequence overlaps with the next sequences
                nextCheck(j);            
                return 1;
            }
            return 0;
        }

         // check if a time sequence overlaps the left side of the one at index i in timeSequences
        int overlapLessCheck(int j, pair<long,long> *timeSeq) {
            if (timeSeq->second >= timeSequences[j]->first && timeSeq->first < timeSequences[j]->first) {
                // combine the sequences due to overlap
                timeSequences[j]->first = timeSeq->first;
                
                // check if the new combined sequence overlaps with the previous sequences
                prevCheck(j);
                return 1;
            }
            return 0;
        }

         // check if a time sequence fits inbetween the sequences at index i and i - 1 in timeSequences 
        int betweenCheck(int j, pair<long,long> *timeSeq) {
            if ((j - 1 > -1 && (timeSeq->second < timeSequences[j]->first && timeSeq->first > timeSequences[j - 1]->second)) || timeSeq->second < timeSequences[j]->first) {
                timeSequences.insert(timeSequences.begin() + j, timeSeq);
                return 1;
            }
            return 0;
        }

    public:
        Station(int id) {
            stationID = id;

            // set the min and max time to flagged values
            minTime = LONG_MAX;
            maxTime = -1;

            timeSequences = vector<pair<long,long>*>();
            
        }

        ~Station() {
            for (const auto& timeSeq : timeSequences) {
                delete timeSeq;
            }
            timeSequences.clear();
        }

        bool operator!=(const Station station) const {
            return stationID != station.stationID;
        }

        bool operator<(const Station station) const {
            return stationID < station.stationID;
        }

        bool operator>(const Station station) const {
            return stationID > station.stationID;
        }

        // find the correct charger then add time to it
        void resolveTimeSequence(long start, long end, bool up) {
            // cout << "size " << timeSequences.size() << "\n";
            if (end < start) {
                throw errorStation("invalid time sequence detected\n");
            }
            
            // cout << "adding time to station " << stationID << "\n";
            // cout << "start " << start << " end " << end << "\n";
            // cout << "curMin " << minTime << " curMax " << maxTime << "\n";
            if (start < minTime) {
                minTime = start;
            }
            if (end > maxTime) {
                maxTime = end;
            }
            // cout << "curMin " << minTime << " curMax " << maxTime << "\n";

            // only need to keep track of sequences where chargers were available
            if (up) {
                // unique_ptr<pair<long,long>> timeSeq = make_unique<pair<long, long>>(start, end);
                pair<long,long> *timeSeq = new pair<long, long>(start, end);

                // if the vector of sequences is empty just insert the new sequence
                if (timeSequences.empty()) {
                    // cout << "there were no time sequences before\n";
                    timeSequences.reserve(1);
                    timeSequences.push_back(timeSeq);
                    return;
                }
 
                // compare the new time sequence against other sequences to check if they can be combined
                // or insert the new sequence into the vector if it does not overlap
                int j;
                for(j = 0; j < timeSequences.size(); j++) {

                    // check if the new sequence underlaps the current one
                    if (underlapCheck(j, timeSeq)) return;
                    // check if the new sequence supercedes the current one
                    if (supercedeCheck(j, timeSeq)) return;
                    // check if the new sequence overlaps with the current sequence's upper bound
                    if (overlapGreaterCheck(j, timeSeq)) return;
                    // check if the new sequence overlaps with the current sequence's lower bound
                    if (overlapLessCheck(j, timeSeq)) return;
                    // check if the new sequence fits inbetween the two sequences around it
                    if (betweenCheck(j, timeSeq)) return;
                }
                // the time sequence was greater than all other ones so put it at the back
                timeSequences.push_back(timeSeq);
                return;
            }
        }

        int getStationID() {
            return stationID;
        }

        // calculate the uptime % of the station by summing the sequences and dividing it by the total time it was in the log
        int getPercentUptime() {
            // cout << "calculating uptime for station " << stationID << "\n";
            long double totalTime = maxTime - minTime;

            long double upTime = 0;
            for(int i = 0; i < timeSequences.size(); i++) {
                // cout << "sequence " << i << " " << timeSequences[i]->first << "," << timeSequences[i]->second << "\n";
                upTime += timeSequences[i]->second - timeSequences[i]->first;
            }

            // cout << "totalTime: " << totalTime << " upTime " << upTime << "\n";

            return (int) ((upTime / totalTime) * 100.0);
        }

        void setStationID(int id) {
            stationID = id;
        }
};

/**
 * =============
 * LogFileProcesser
 * =============
 * - parses a given log file
 * - reads a given file line by line
 * - determines the uptime of all the stations within the log file
*/
class LogFileProcesser {
    private:
        map<int,Station*> stations;
        string currentLogLine = "";
        ifstream logFile;

        // reach a specified delimiter within the file
        void getToHeader(string header) {
            do {
                if (currentLogLine.compare(header) == 0) {
                    break;
                }
            } while(getline(logFile,currentLogLine));
        }

        // parse the first section of the log file
        // to keep track of each station and charger
        int processStations() {
            // get to the header for all the stations
            getToHeader(STATION_HEADER);

            while(getline(logFile,currentLogLine)) {
                // check for the end of this section
                if (currentLogLine.compare("") == 0) {
                    break;
                } else {
                    stringstream ss(currentLogLine);
                    string token;
                    int stationID = -1;
                    Station *newStation = new Station(stationID);

                    while(getline(ss,token, ' ')) {
                        int id;
                        
                        try {
                            id = stoi(token);
                        } catch (exception e) {
                            // there was not a valid id
                            throw errorStation("invalid format of station or charger id\n");
                            return 0;
                        }
                        
                        if (stationID < 0) {
                            // only the first token is the station
                            stationID = id; 
                            // cout << "making new station with id " << id << "\n";
                            newStation->setStationID(stationID);
                        } else if (stations.find(id) == stations.end()) {
                            // otherwise it is a charger within that station
                            
                            stations[id] = newStation;
                            // cout << "charger " << id << " assigned to station " << newStation << "\n";
                        } else {
                            throw errorStation("duplicate or invalid station or charger id\n");
                        }
                    }
                }
            }
            // everything was read correctly
            return 1;
        }

        // parse the second section of the log file
        // to track the uptime of each station and charger
        int processAvailabilityReports() {
            // cout << "processing reports\n";
            // get to the header for all the reports
            getToHeader(AVAILABILITY_REPORT_HEADER);
            
            while(getline(logFile,currentLogLine)) {
                // cout << "processing " << currentLogLine << "\n";
                // check for the end of this section
                if (currentLogLine.compare("") == 0) {
                    break;
                } else {

                    stringstream ss(currentLogLine);
                    string token;
                    vector<string> tokens;

                    // parse the single report into tokens
                    while(getline(ss,token, ' ')) {
                        tokens.push_back(token);
                    }

                    try {
                        // each line in the report has 4 values
                        int id = stoi(tokens[0]);
                        long start = stol(tokens[1]);
                        long end = stol(tokens[2]);
                        bool up = tokens[3].compare("true") == 0 ? true : false;

                        // cout << "adding time to station " << stations[id] << "\n";
                        stations[id]->resolveTimeSequence(start,end,up);
                        // cout << "added time\n";
                    } catch (exception e) {
                        // one or more of the values was not in the expected format
                        throw errorStation("invalid format of charger availability report\n");
                        return 0;  
                    }
                }

            }
            // everything was read correctly
            return 1;
        }

    public:

        ~LogFileProcesser() {
            for (const auto& pair : stations) {
                delete pair.second;
            }
            stations.clear();
        }

        // attempts to read the file and parse it
        string processFile(string filePath) {
            
            try {   
                logFile = ifstream(filePath);
            } catch (exception e) {
                return "ERROR\n";
            }
            
            // read all the stations
            if (!processStations()) {
                return "ERROR\n";
            }

            // read all the reports
            if (!processAvailabilityReports()) {
                return "ERROR\n";
            }

            set<int> uniqueStations;

            // make a string in the desired format
            string uptimeReport = "";
            for (const auto& pair : stations) {   
                // cout << "getting uptime for station: " << stations[i].getStationID() << "\n";
                int id = pair.second->getStationID();
                // cout << "looking at charger " << pair.first << " associated with station " << id <<"\n";
                if(uniqueStations.find(id) == uniqueStations.end()) {
                    int uptime = pair.second->getPercentUptime();
                    // cout << "uptime of station " << uptime << "\n";
                    uptimeReport += to_string(id) + " " + to_string(uptime) + "\n";
                }         
                uniqueStations.insert(pair.second->getStationID());       
            }

            return uptimeReport;
        }

       
};


/**
 * =============
 * UnitTest
 * =============
 * Runs unit tests on:
 * - stations
*/
class UnitTest {
    public:

        // test to ensure the stations calculate the correct percent of uptime
        static bool test1() {
            // define the chargers and stations
            Station station1 = Station(1);
            Station station2 = Station(2);

            // add time to the chargers
            station1.resolveTimeSequence(25000,50000,true);
            station1.resolveTimeSequence(27000,90900,true);

            station2.resolveTimeSequence(25000,50000,false);
            station2.resolveTimeSequence(27000,90900,false);

            // check if the calculated time is correct
            return station1.getPercentUptime() == 100 && station2.getPercentUptime() == 0;
        }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "ERROR\n";
        return 1;
    }
    
    LogFileProcesser processor;

    try {
        string uptimeReport = processor.processFile(argv[1]); 
        cout << uptimeReport;
    } catch (errorStation e) {
        cout << "ERROR\n";
        cerr << e.what();
    }
    
    

    // unit tests for charger and station classes

    // bool test1 = UnitTest::test1();

    // cout << "test1 results: " << test1 << "\n";
    

    return 0;



}