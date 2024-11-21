
void getRunDuration(std::vector<int> Runs = {544122}) {
    // Initialize the CCDB manager instance
    auto& cc = o2::ccdb::BasicCCDBManager::instance();

     // Open a text file for writing the result
    std::ofstream outfile("run_duration_544122.txt", std::ios::out);
    if (!outfile.is_open()) {
        std::cerr << "Error opening file for writing!" << std::endl;
        return;
    }
    outfile <<"# RUN START END"<< std::endl;
    for(int Run : Runs){
    // Get the run duration as an std::pair
    auto duration = cc.getRunDuration(Run);

    // Write the std::pair to the file
    outfile << Run << " " <<  duration.first <<" "<< duration.second << std::endl;    
    }
    // Print a confirmation message
    std::cout << "Run duration written to run_duration.txt" << std::endl;

    outfile.close();
}