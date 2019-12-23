#include "Stdf_parser.h"
#include <iostream>
#include "string_utils.h"

//TODO: Clear up all other variables
Stdf_parser::~Stdf_parser()
{
    stdf_fh.close();
}


int Stdf_parser::read_header(header_t& header)
{

    if (stdf_fh.tellg() >= file_len) {
        return FILE_EOF;
    }


    char header_buffer[4];
    int address = stdf_fh.tellg();
    stdf_fh.read(header_buffer, HEADER_LEN);
    if ( stdf_fh.gcount() != HEADER_LEN ) {
        return FILE_ERROR_HEADER;
    } 
    
    int length = ( ( header_buffer[1] & 0xFF ) << 8)  | (header_buffer[0] & 0xFF );
    int type = header_buffer[2];
    int sub_type = header_buffer[3];

    char *data_buffer = new char[length];
    //TODO: check if buffer was allocated.

    stdf_fh.read(data_buffer, length);
    delete [] data_buffer;
    if  (stdf_fh.gcount() != length) {
        return FILE_ERROR_DATA;
    }

    header.address = address;
    header.len = length;
    header.type = type;
    header.sub_type = sub_type;

    return 0;
}

void Stdf_parser::read_data(const header_t& header,char *buffer)
{
    stdf_fh.seekg(header.address + 4);
    stdf_fh.read(buffer, header.len);

}

void Stdf_parser::stdf_parser_init()
{
    //this->stdf_filename = stdf_filename;

    stdf_fh.open(this->stdf_filename.c_str(), ios::binary | ios::ate);    
    if (!stdf_fh.good()) {
        stdf_parser_state = 1;
        return;
    }  
    //doesnt seem to work.
    file_len = stdf_fh.tellg();
    stdf_fh.seekg(0);

    headers.clear();
    header_t header;
    int header_status;
    int rec_id = 0;
    
    while ((header_status = read_header(header))  == 0) {
        header.rec_id = rec_id++;
        header.rec_name = get_rec_name(header);
        headers.push_back(header);
    }
    record_count = rec_id;

    if (header_status != FILE_EOF) {
        stdf_parser_state = header_status;
    } else {
        stdf_parser_state = 0;
    }

}

char *Stdf_parser::allocate_data_buffer(const header_t& header)      
{                                                       
    char *data = new char[header.len];                  
    return data;                                        
}                                                       

string Stdf_parser::get_rec_name(const header_t& header) 
{
    string rec_name = "UNKNOWN";
    switch (header.type) {
        case 0:
            switch (header.sub_type) {
                case 10: {
                    rec_name="FAR";
                    break;
                }
                case 20:
                    rec_name="ATR";
                    break;
            }
            break;
        case 1:
            switch(header.sub_type) {
                case 10:
                    rec_name="MIR";
                    break;
                case 20:
                    rec_name="MRR";
                    break;
                case 30:
                    rec_name="PCR";
                    break;
                case 40:
                    rec_name="HBR";
                    break;
                case 50:
                    rec_name="SBR";
                    break;
                case 60:
                    rec_name="PMR";
                    break;
                case 62:
                    rec_name="PGR";
                    break;
                case 63:
                    rec_name="PLR";
                    break;
                case 70:
                    rec_name="RDR";
                    break;
                case 80:
                    rec_name="SDR";
                    break;
            }
            break;
        case 2:
            switch(header.sub_type) {
                case 10:
                    rec_name="WIR";
                    break;
                case 20:
                    rec_name="WRR";
                    break;
                case 30:
                    rec_name="WCR";
                    break;
            }
            break;
        case 5:
            switch(header.sub_type) {
                case 10:
                    rec_name="PIR";
                    break;
                case 20:
                    rec_name="PRR";
                    break;
            }
            break;
        case 10:
            switch(header.sub_type) {
                case 30:
                    rec_name="TSR";
                    break;
            }
            break;
        case 15:
            switch(header.sub_type) {
                case 10:
                    rec_name="PTR";
                    break;
                case 15:
                    rec_name="MPR";
                    break;
                case 20:
                    rec_name="FTR";
                    break;
            }
            break;
        case 20:
            switch(header.sub_type) {
                case 10:
                    rec_name="BPS";
                    break;
                case 20:
                    rec_name="EPS";
                    break;
            }
            break;
        case 50:
            switch(header.sub_type) {
                case 10:
                    rec_name="GDR";
                    break;
                case 20:
                    rec_name="DTR";
                    break;
            }
            break;
    }
    return rec_name;    
}


BASE Stdf_parser::decode_stdf_record(const header_t& header) 
{
    BASE B; 
    char *buffer = allocate_data_buffer(header);
    read_data(header, buffer);

    switch (header.type) {
        case 0:
            switch (header.sub_type) {
                case 10: {
                    B= FAR( (unsigned char *) buffer);
                    break;
                }
                case 20:
                    B= ATR( (unsigned char *) buffer);
                    break;
            }
            break;
        case 1:
            switch(header.sub_type) {
                case 10:
                    B= MIR( (unsigned char *) buffer);
                    break;
                case 20:
                    B= MRR( (unsigned char *) buffer);
                    break;
                case 30:
                    B= PCR( (unsigned char *) buffer);
                    break;
                case 40:
                    B= HBR( (unsigned char *) buffer);
                    break;
                case 50:
                    B= SBR( (unsigned char *) buffer);
                    break;
                case 60:
                    B= PMR( (unsigned char *) buffer);
                    break;
                case 62:
                    B= PGR( (unsigned char *) buffer);
                    break;
                case 63:
                    B= PLR( (unsigned char *) buffer);
                    break;
                case 70:
                    B= RDR( (unsigned char *) buffer);
                    break;
                case 80:
                    B= SDR( (unsigned char *) buffer);
                    break;
            }
            break;
        case 2:
            switch(header.sub_type) {
                case 10:
                    B= WIR( (unsigned char *) buffer);
                    break;
                case 20:
                    B= WRR( (unsigned char *) buffer);
                    break;
                case 30:
                    B= WCR( (unsigned char *) buffer);
                    break;
            }
            break;
        case 5:
            switch(header.sub_type) {
                case 10:
                    B= PIR( (unsigned char *) buffer);
                    break;
                case 20:
                    B= PRR( (unsigned char *) buffer);
                    break;
            }
            break;
        case 10:
            switch(header.sub_type) {
                case 30:
                    B= TSR( (unsigned char *) buffer);
                    break;
            }
            break;
        case 15:
            switch(header.sub_type) {
                case 10:
                    B= PTR( (unsigned char *) buffer);
                    break;
                case 15:
                    B= MPR( (unsigned char *) buffer);
                    break;
                case 20:
                    B= FTR( (unsigned char *) buffer);
                    break;
            }
            break;
        case 20:
            switch(header.sub_type) {
                case 10:
                    B= BPS( (unsigned char *) buffer);
                    break;
                case 20:
                    B= EPS( (unsigned char *) buffer);
                    break;
            }
            break;
        case 50:
            switch(header.sub_type) {
                case 10:
                    B= GDR( (unsigned char *) buffer);
                    break;
                case 20:
                    B= DTR( (unsigned char *) buffer);
                    break;
            }
            break;
    }
    
    delete[] buffer; 
    return B;
}


void Stdf_parser::stdf_as_text()
{
    vector<string> records;
    for ( vector<header_t>::iterator header_it = headers.begin(); header_it != headers.end(); header_it++) {
        records.clear();
        record_as_text(*header_it, records);
    }
}

void Stdf_parser::record_as_text(const header_t& header, vector<string>& records, bool quiet)
{
        BASE b = decode_stdf_record(header);
        vector<string> field_names = b.get_field_names();
        for (vector<string>::iterator field_name_it=field_names.begin(); field_name_it != field_names.end(); field_name_it++) {
            if (quiet == false) {
                cout << header.rec_id << ":" << b.get_rec_name() << ":" <<  *field_name_it << ":" <<  b.get_data(*field_name_it) << endl;
            }
            records.push_back(to_string( (long long) (header.rec_id)) + " : " + b.get_rec_name() + " : " +  *field_name_it + " : " +  b.get_data(*field_name_it)); 
        }
        if (quiet == false) {
            cout << "-------------------" << endl;
        }
}

void Stdf_parser::records_as_text(const vector<header_t>& filtered_headers)
{
        vector<string> records;
        for ( vector<header_t>::const_iterator it=filtered_headers.begin(); it != filtered_headers.end(); it++ ) {
            records.clear();
            record_as_text(*it, records);
        }
}

vector<header_t> Stdf_parser::get_stdf_headers_of_type(string rec_names, int rec_id_start, int rec_id_end)
{
    bool rec_name_always_match = false;
    vector<string> _rec_names = split(rec_names, ",");
    if (_rec_names.empty() ) {
        rec_name_always_match = true;
    }
    //for faster lookup.
    map<string, int> rec_names_to_look_for;
    for (vector<string>::iterator it=_rec_names.begin(); it!= _rec_names.end(); it++) {
        rec_names_to_look_for[*it] = 1;
    }

    vector<header_t> filtered_headers;
    if (rec_id_end == 0 || rec_id_end > record_count || rec_id_end < 0) {
        rec_id_end = record_count;
    }
    if (rec_id_start < 0) {
        rec_id_start = 0;
    }

    for (vector<header_t>::iterator it=headers.begin(); it!=headers.end(); it++) {
        header_t header = *it;
       
        string rec_name=header.rec_name;
        int rec_id = header.rec_id; 

        map<string, int>::iterator iter=rec_names_to_look_for.find(rec_name);
        if (!( rec_name_always_match || iter != rec_names_to_look_for.end() )) { 
            continue;
        }

        if (!( (rec_id >= rec_id_start) && (rec_id <= rec_id_end ) )) {
            continue;
        }
        filtered_headers.push_back(header);
    }
    return filtered_headers;
}

#ifdef __STDF_PARSER_TEST__

#include <iostream>
using namespace std;

int main(int argc, char *argv[])
{
    if (argc < 5) {
        cout << "Usage: " << argv[0] << " <stdf_filename> <rec_names, 'all' to retrieve all, else comma-delimited rec_names> <rec_id_start, -1 for none> <rec_id_end, -1 for none>" << endl;
        cout << "Example(get PIR,PRR with recid from 1 to 100): " << argv[0] << " sample.stdf PIR,PRR, 1 100" << endl;
        cout << "Example(get all records with recid from 5 to end : " << argv[0] << " sample.stdf all 5 -1" << endl;
        cout << "Example(get all MIR with recid from start to end : " << argv[0] << " sample.stdf MIR 0 -1" << endl;
        return 1;
    }


    string stdf_filename(argv[1]); 
    string rec_names(argv[2]);
    if (rec_names == "all") {
        rec_names = "";
    }
    int    rec_id_start = atoi(argv[3]);
    int    rec_id_end   = atoi(argv[4]);


    Stdf_parser parser(stdf_filename); 
    int stdf_parser_state = parser.get_stdf_parser_state();
    
    if (stdf_parser_state) { 
        cout << "Error: Stdf parser failed to initialize. error code=" << stdf_parser_state << endl;
        return 1;
    }
    //parser.stdf_as_text(); 
    vector<header_t> filtered_headers = parser.get_stdf_headers_of_type(rec_names, rec_id_start, rec_id_end);    

    //the following lines can be accomplished by calling records as text.
    //the only difference is that for records_as_text, it doesnt save the string. 
    vector<string> records;
    for (vector<header_t>::iterator it=filtered_headers.begin(); it!=filtered_headers.end(); it++) {
        records.clear();
        parser.record_as_text(*it, records, true);
        for (vector<string>::iterator iter=records.begin(); iter!=records.end(); iter++) {
            cout << *iter << endl;    
        }
        cout << "-------------------" << endl;
    }
    //parser.records_as_text(filtered_headers);

    return 0;
}
#endif
