#include "sniffer2.h"

// παιρνει τον buffer και κραταει μονο το filename απομακρυνοντας τους υπολοιπους χαρακτηρες 
string clean_buffer(string filename){
    char achar;
    string filename2 = "";
    
    for(int i = 0; i<filename.length(); i++){ 
        achar = filename.at(i);
        if(achar != ' ' && achar != '\n' && achar != '\0'){
            filename2.push_back(achar);
        }
    }

    return filename2;

}

//parser + δημιουργια αρχειου .out
map<string,int> parser(string filename){
    
    filename = clean_buffer(filename);

    string *allhttps = new string[1000];
    fstream httpfile;
    string str;
    int num_of_https = 0;
    string ex_dir = "./ex_directory/";
    ex_dir = ex_dir + filename;
    httpfile.open(ex_dir, ios::in); // ανοιξε το αρχειο που σου εδωσα
    map<string, int> https;


    while (httpfile >> str) //για καθε string
    {
        int ll = str.length();
        string ht = "http://";
        string www = "www.";

        if (ll <= 7) //σημαινει οτι δεν χωραει ουτε το http:// αρα προχωρα
        {
            continue;
        }

        string ht2 = "";
        for (int i = 0; i < 7; i++)
            ht2.push_back(str.at(i)); // αν ειναι link οι πρωτοι 7 χαρακτηρες θα ναι http://

        if (ht.compare(ht2) != 0)   
            continue;

        int length = 7;

        string www2 = ""; //αν περιεχει www μην το κρατησεις
        for (int i = 7; i < 11; i++)
            www2.push_back(str.at(i));

        if (www.compare(www2) == 0)
            length = 11;

        char ch = str.at(length);
        string mypart = "";
        while (ch != '/' && ch != ' ' && length < ll) 
        {
            ch = str.at(length);
            if(ch != '/' && ch != ' '){ // το λινκ , μεχρι να βρεις τελος ή /
                mypart.push_back(ch);
                length++;
            }
        }

        allhttps[num_of_https] = mypart; //ολα τα link σε πινακα
        num_of_https++;
    }

    https.insert({allhttps[0], 1}); // μετατρπω τον πινακα με τα λινκ σε map
    for (int i = 0; i < num_of_https; i++)
    {
        auto srch = https.find(allhttps[i]);
        if (srch != https.end())
        {
            int anum = srch->second + 1;
            https[allhttps[i]] = anum;
        }
        else
        {
            https.insert({allhttps[i], 1});
        }
    }

    string ex_directory = "out_directory/"; // sent the .out file in out_directory
    string ht = ex_directory + filename;
    string out = ".out"; // με την καταληξη .out
    ht = ht + out;

    ofstream outfile(ht);  // in my new .out file

    map<string, int>::iterator itr;
    for (itr = https.begin(); itr != https.end(); itr++)
        outfile << itr->first << " " << itr->second << endl; // paste every key and value from map

    return https;
}