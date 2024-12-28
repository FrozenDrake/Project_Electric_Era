// Seth Pavlicek

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;

const string STATION_HEADER = "[Stations]";
const string AVAILABILITY_REPORT_HEADER = "[Charger Availability Reports]";

/**
 * =============
 * Charger
 * =============
 * - manages the uptime for a single charger
 * - uses given start and end time of each report to calculate the uptime
*/
class Charger {
    private:
        int chargerID;

        long upTime;
        long downTime;

        long lastKnownTime;

    public:
        Charger(int chargerID) {
            this->chargerID = chargerID;

            upTime = 0;
            downTime = 0;

            // -1 indicates uninitialized
            lastKnownTime = -1;
        }

        void addTime(long start, long end, bool up) {
            //add to the downtime if the was a gap of unrecorded time in the log file
            if (lastKnownTime < 0) {
                lastKnownTime = start;
            }
            downTime += (start - lastKnownTime);
            lastKnownTime = end;

            long time = end - start;

            //add the time respectively
            if (up) {
                upTime += time;
            } else {
                downTime += time;
            }
        }

        int getChargerID() {
            return chargerID;
        }

        // calculate what % of the time the charger was up by dividing the uptime by the total time
        int getPercentUptime() {
            float percent =  ((float) upTime/ (float) (upTime + downTime));
            percent *= 100.0;
            return (int) percent;
        }

};

/**
 * =============
 * Station
 * =============
 * - manages the uptime for a single station
 * - calculates the uptime for a station
 * - takes the average of all its chargers
*/
class Station {
    private:
        vector<Charger> chargers;
        int stationID;

    public:
        Station(int id) {
            stationID = id;
        }

        void addNewCharger(int chargerID) {
            chargers.push_back(Charger(chargerID));
        }

        // find the correct charger then add time to it
        void addTimeToCharger(int chargerID, long start, long end, bool up) {
            for(int i = 0; i < chargers.size(); i++) {
                if (chargers[i].getChargerID() == chargerID) {
                    // cout << "found charger " << chargerID << "\n";
                    chargers[i].addTime(start, end, up);

                    return;
                }
            }

        }

        int getStationID() {
            return stationID;
        }

        // calculate the uptime % of the station by finding the average uptime % of all its chargers
        int getPercentUptime() {

            int sum = 0;
            for(int i = 0; i < chargers.size(); i++) {
                sum += chargers[i].getPercentUptime();
            }

            return sum / chargers.size();
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
        vector<Station> stations;
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
                    bool newStation = true;

                    while(getline(ss,token, ' ')) {
                        int id;
                        try {
                            id = stoi(token);
                        } catch (exception e) {
                            // there was not a valid id
                            return 0;
                        }
                        
                        if (newStation) {
                            // only the first token is the station
                            stations.push_back(Station(id));
                            newStation = false;
                        } else {
                            // otherwise it is a charger within that station
                            // because "push_back()" is being used the newest station
                            // is at the back of the list
                            stations.back().addNewCharger(id);
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
            // get to the header for all the reports
            getToHeader(AVAILABILITY_REPORT_HEADER);
            
            while(getline(logFile,currentLogLine)) {
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

                        for (int i = 0; i < stations.size();i++) {
                            stations[i].addTimeToCharger(id, start, end, up);
                        }
                    } catch (exception e) {
                        // one or more of the values was not in the expected format
                        return 0;  
                    }
                }

            }
            // everything was read correctly
            return 1;
        }

    public:
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

            // make a string in the desired format
            string uptimeReport = "";
            for (int i = 0; i < stations.size();i++) {
                uptimeReport += to_string(stations[i].getStationID()) + " " + to_string(stations[i].getPercentUptime()) + "\n";
            }

            return uptimeReport;
        }

       
};


/**
 * =============
 * UnitTest
 * =============
 * Runs unit tests on:
 * - chargers
 * - stations
*/
class UnitTest {
    public:
        // test to ensure the charger calculate the correct percent of uptime
        static bool test1() {
            // define the chargers
            Charger charger1 = Charger(1);
            Charger charger2 = Charger(2);

            charger1.addTime(60000,127823,true);
            charger2.addTime(0,1878,false);

            return charger1.getPercentUptime() == 100 && charger2.getPercentUptime() == 0;
        }

        // test to ensure the stations calculate the correct percent of uptime
        static bool test2() {
            // define the chargers and stations
            Station station1 = Station(1);
            Station station2 = Station(2);

            Charger charger1 = Charger(1);
            Charger charger2 = Charger(2);
            Charger charger3 = Charger(3);
            Charger charger4 = Charger(4);     

            // add the chargers to the stations
            station1.addNewCharger(1);
            station1.addNewCharger(2);

            station2.addNewCharger(3);
            station2.addNewCharger(4);

            // add time to the chargers
            station1.addTimeToCharger(1,25000,50000,true);
            station1.addTimeToCharger(2,27000,90900,true);

            station2.addTimeToCharger(3,25000,50000,false);
            station2.addTimeToCharger(4,27000,90900,false);

            // check if the calculated time is correct
            return station1.getPercentUptime() == 100 && station2.getPercentUptime() == 0;
        }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "ERROR";
        return 1;
    }

    LogFileProcesser processor;

    string uptimeReport = processor.processFile(argv[1]); 
    cout << uptimeReport;

    /* unit tests for charger and station classes

    bool test1 = UnitTest::test1();
    bool test2 = UnitTest::test2();

    cout << test1 << " " << test2;
    */

    return 0;



}