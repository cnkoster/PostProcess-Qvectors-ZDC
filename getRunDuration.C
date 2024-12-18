
void getRunDuration(std::vector<int> Runs = {544091, 544095, 544098, 544116, 544121, 544122, 544123, 544124}) {
    // Initialize the CCDB manager instance
    auto& cc = o2::ccdb::BasicCCDBManager::instance();

     // Open a text file for writing the result
    
    for(int Run : Runs){
        std::ofstream outfile(Form("run_duration_%i.txt", Run), std::ios::out);
        if (!outfile.is_open()) {
            std::cerr << "Error opening file for writing!" << std::endl;
            return;
        }
        outfile <<"# RUN START END"<< std::endl;
        // Get the run duration as an std::pair
        auto duration = cc.getRunDuration(Run);

        // Write the std::pair to the file
        outfile << Run << " " <<  duration.first <<" "<< duration.second << std::endl;    
        outfile.close();
    }
    // Print a confirmation message
    std::cout << "Run duration written" << std::endl;
}