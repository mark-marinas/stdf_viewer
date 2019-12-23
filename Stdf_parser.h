#ifndef __STDF_PARSER__
#define __STDF_PARSER__

#include <string>
#include <fstream>
#include <vector> 
#include <iostream>
#include <sstream>
#include <map>
#include <cstdint>
#include <cmath>
#include <iomanip> 
#include <ctime>

using namespace std; 

#define HEADER_LEN          4
#define FILE_EOF            1
#define FILE_ERROR_HEADER   2
#define FILE_ERROR_DATA   (FILE_ERROR_HEADER + 1)

#define SIZEOF_FIELD_NAMES ( sizeof(field_names)/sizeof(field_names[0]) )
#define ADD_TO_FIELD_NAMES do { for (int i=0; i<SIZEOF_FIELD_NAMES; i++ ) { this->field_names.push_back(field_names[i]); } } while(0);


typedef uint32_t    stdf_date_t;
typedef unsigned    char stdf_uchar_t;
typedef char        stdf_char_t;
typedef uint16_t    stdf_uint2_t;

struct header_t {
    int address;
    int len;
    int type, sub_type;
    string rec_name;
    int rec_id;
};

class BASE {
    protected:
        map<string, string> data;
        vector<string> field_names;
        string rec_name;
    public:
        vector<string> get_field_names() { 
            return field_names;
        };

        string get_data(const string& name) {
            map<string,string>::iterator it = data.find(name);
            if (it != data.end()) {
                return it->second;
            }
            return "";
        };
        ~BASE() { 
            data.clear();
            field_names.clear(); 
        };

        string get_rec_name() { return rec_name; };

        string uchar_to_binary_string(unsigned char *buffer, int len, int cpu_ver=0)
        {
            string binary_string = "";
            for (int i=0; i<len; i++) {
                string current_buffer_string = "";
                for (int j=7; j>=0; j--) {
                    if ( ( ( ( unsigned int ) *buffer >> j ) & 0x01 ) == 1) {
                        current_buffer_string += "1";
                    } else {
                        current_buffer_string += "0";
                    }
                }
                binary_string = current_buffer_string + binary_string;
                buffer++;
            }
            return binary_string;
        }

        string intdate_to_string(stdf_date_t int_date)
        {
            char char_time[20];
            time_t temp = (uint32_t) int_date;
            tm  *utc_time = localtime(&temp);
            strftime(char_time,sizeof(char_time),"%H:%M:%S %Y/%m/%d", utc_time);

            return string(char_time);
        }

        //TODO: This should be base on cpu_ver
        unsigned int uchar_to_int(unsigned char *buffer, int len, int cpu_ver=0)
        {
            unsigned int retval = 0;
            for (int i=0;i<len; i++) {
                retval = retval | ( *buffer++ << (i*8) );
            }

            return retval;
        }

        float uchar_to_float(unsigned char *buffer, int cpu_ver=0) {
            uint32_t decimal = 0;
            for (int i=0;i<4; i++) {
                decimal = decimal | (uint32_t) ( *buffer++ << (i*8) );
            }
            int sign_bit =  (decimal & 0x80000000)?-1:1;
            int exp_bits = ( decimal >> 23 ) & 0xFF;
            int mantissa_bits = (decimal & 0x7FFFFF);

            float mantissa = 1;
            for (int i=1;i<=23; i++) {
                mantissa += ((mantissa_bits >> (23-i)) & 0x01)*pow(2,-1*(i));
            }
            return sign_bit*(pow(2,exp_bits-127)) * mantissa;
        }

        double uchar_to_float64(unsigned char *buffer, int cpu_ver=0) {
            uint64_t decimal = 0;
            for (int i=0;i<8; i++) {
                decimal = decimal | (uint64_t) ( *buffer++ << (i*8) );
            }
            int sign_bit =  (decimal & 0x8000000000000000)?-1:1;
            int exp_bits = ( decimal >> 52 ) & 0x7FF;
            int mantissa_bits = (decimal & 0x7FFFFFFFFFFFF);

            double mantissa = 1;
            for (int i=1;i<=52; i++) {
                mantissa += ((mantissa_bits >> (52-i)) & 0x01)*pow(2,-1*(i));
            }
            return sign_bit*(pow(2,exp_bits-1023)) * mantissa;
        }


};

class FAR: public BASE {
    public:
        FAR(unsigned char *buffer) {
            rec_name="FAR";
            data["CPU_TYP"]  = to_string((long long) buffer[0]);
            data["STDF_VER"] = to_string((long long) buffer[1]);
        
            field_names.push_back("CPU_TYP");
            field_names.push_back("STDF_VER");
        };
};

class ATR: public BASE {
    public:
        ATR(unsigned char *buffer) {
            rec_name="ATR";
            stdf_date_t date = 0;
            string cmd_line = "";
            date = uchar_to_int(buffer, 4);
            buffer += 4; 

            int len=*buffer++; 
            for (int i=0; i<len; i++) {
                cmd_line += *buffer++; 
            } 

            data["MOD_TIM"] = intdate_to_string(date); 
            data["CMD_LINE"] = cmd_line ;

            field_names.push_back("MOD_TIM");
            field_names.push_back("CMD_LINE");
        };
};

class MIR: public BASE {
    public:
        MIR(unsigned char *buffer) {
            rec_name="MIR";
            stdf_date_t setup_t, start_t;
            stdf_uint2_t burn_tim;

            string keys[] = {
                "LOT_ID","PART_TYP", "NODE_NAM", "TSTR_TYP", "JOB_NAM", "JOB_REV", "SBLOT_ID",
                "OPER_NAM", "EXEC_TYP", "EXEC_VER", "TEST_COD", "TST_TMP", "USER_TXT", "AUX_FILE", 
                "PKG_TYP", "FAMLY_ID", "DATE_COD", "FACIL_ID", "FLOOR_ID", "PROC_ID", "OPER_FRQ",
                "SPEC_NAM", "SPEC_VER", "FLOW_ID", "SETUP_ID", "DSGN_REV", "ENG_ID", "ROM_COD", "SERL_NUM",
                "SUPR_NAM"
            };

            setup_t = uchar_to_int(buffer, 4); buffer += 4;
            data["SETUP_T"] = intdate_to_string(setup_t);
            field_names.push_back("SETUP_T");

            start_t = uchar_to_int(buffer, 4); buffer += 4;
            data["START_T"] = intdate_to_string(start_t);
            data["STAT_NUM"] = to_string( (long long)*buffer++);
            data["MODE_COD"] = *buffer++; 
            data["RTST_COD"] = *buffer++; 
            data["PROT_COD"] = *buffer++; 

            field_names.push_back("START_T");
            field_names.push_back("STAT_NUM");
            field_names.push_back("MODE_COD");
            field_names.push_back("RTST_COD");
            field_names.push_back("PROT_COD");

            burn_tim = uchar_to_int(buffer, 2); buffer += 2;
            data["BURN_TIM"] = to_string( (long long)burn_tim);
            data["CMOD_COD"] = *buffer++; //to_string( (long long)*buffer++);

            field_names.push_back("BURN_TIM");
            field_names.push_back("CMOD_COD");

            for (int i=0; i<30; i++) { 
                string key = keys[i];
                field_names.push_back(key);
                int len=*buffer++;
                for (int j=0;j<len; j++) {
                    data[key] += *buffer++;
                }
            }
        };
};

class MRR: public BASE {
    public:
        MRR(unsigned char *buffer) {
            rec_name="MRR";
            stdf_date_t finish_time;
            finish_time = uchar_to_int(buffer, 4); buffer += 4;
            data["FINISH_T"] = intdate_to_string(finish_time);
            data["DISP_COD"] = *buffer++; 
            field_names.push_back("FINISH_T");
            field_names.push_back("DISP_COD");

            string keys[] = { "USR_DESC", "EXC_DESC" };             
            for (int i=0; i<2; i++) {
                string key=keys[i];
                field_names.push_back(key);
                int len = *buffer;
                for (int j=0; j<len; j++) {
                    data[key] += *buffer++;
                }
            }
        };

};

class PCR: public BASE {
    public:
        PCR(unsigned char *buffer) {
            rec_name="PCR";
            data["SITE_NUM"] = to_string( (long long) *buffer++);
            data["HEAD_NUM"] = to_string( (long long) *buffer++);
            field_names.push_back("HEAD_NUM");
            field_names.push_back("SITE_NUM");
            string keys[] = { "PART_CNT", "RTST_CNT", "ABRT_CNT", "GOOD_CNT", "FUNC_CNT" };
            for (int i=0; i<5; i++) {
                string key=keys[i];
                unsigned int count=uchar_to_int(buffer, 4); buffer += 4;
                data[key] = to_string( (long long int) count);
                field_names.push_back(key);
            }
        };

};

class HBR: public BASE {
    public:
        HBR(unsigned char *buffer) {
            rec_name="HBR";
            data["HEAD_NUM"] = to_string( (long long) *buffer++);
            data["SITE_NUM"] = to_string( (long long) *buffer++);
            data["HBIN_NUM"] = to_string( (long long) (uchar_to_int(buffer,2)));buffer += 2;
            data["HBIN_CNT"] = to_string( (long long) (uchar_to_int(buffer,4)));buffer += 4;
            data["HBIN_PF"] = *buffer++;
            int len=*buffer++;
            for (int i=0; i<len; i++) {
                data["HBIN_NAM"] += *buffer++;
            } 
            field_names.push_back("HEAD_NUM");
            field_names.push_back("SITE_NUM");
            field_names.push_back("HBIN_NUM");
            field_names.push_back("HBIN_CNT");
            field_names.push_back("HBIN_PF");
            field_names.push_back("HBIN_NAM");
        };
};

class SBR: public BASE {
    public:
        SBR(unsigned char *buffer) {
            rec_name="SBR";
            data["HEAD_NUM"] = to_string( (long long) *buffer++);
            data["SITE_NUM"] = to_string( (long long) *buffer++);
            data["SBIN_NUM"] = to_string( (long long) (uchar_to_int(buffer,2)));buffer += 2;
            data["SBIN_CNT"] = to_string( (long long) (uchar_to_int(buffer,4)));buffer += 4;
            data["SBIN_PF"] = *buffer++;
            int len=*buffer++;
            for (int i=0; i<len; i++) {
                data["SBIN_NAM"] += *buffer++;
            } 
            field_names.push_back("HEAD_NUM");
            field_names.push_back("SITE_NUM");
            field_names.push_back("SBIN_NUM");
            field_names.push_back("SBIN_CNT");
            field_names.push_back("SBIN_PF");
            field_names.push_back("SBIN_NAM");
        };
};

class PMR: public BASE {
    public:
        PMR(unsigned char *buffer) {
            rec_name="PMR";
            string field_names[] = {
                "PMR_INDX", "CHAN_TYP", "CHAN_NAM", "PHY_NAM", "LOG_NAM", "HEAD_NUM", "SITE_NUM"
            };
            
            data[field_names[0]] = to_string( (long long) uchar_to_int(buffer, 2)); buffer+=2;
            data[field_names[1]] = to_string( (long long) uchar_to_int(buffer, 2)); buffer+=2;

            int len=*buffer++; 
            data[field_names[2]] = "";
            for (int i=0;i<len; i++) {
                data[field_names[2]] += *buffer++; 
            }

            len=*buffer++; 
            data[field_names[3]] = "";
            for (int i=0;i<len; i++) {
                data[field_names[3]] += *buffer++; 
            }

            len=*buffer++; 
            data[field_names[4]] = "";
            for (int i=0;i<len; i++) {
                data[field_names[4]] += *buffer++; 
            }

            data[field_names[5]] = to_string((long long)*buffer++);
            data[field_names[6]] = to_string((long long)*buffer++);

            ADD_TO_FIELD_NAMES
        };
};

class PGR:public BASE {
    public:
        PGR(unsigned char *buffer) {
            rec_name="PGR";
            string field_names[] = { "GRP_INDX", "GRP_NAM", "INDX_CNT", "PMR_INDX" };
            data[field_names[0]] = to_string( (long long) uchar_to_int(buffer, 2)); buffer +=2 ;
            int len=*buffer++;
            for (int i=0;i<len; i++) {
                data[field_names[1]] += *buffer++;
            }
            int indx_cnt = uchar_to_int(buffer, 2); buffer += 2;
            data[field_names[2]] = to_string( (long long) indx_cnt);

            if (indx_cnt > 0) {
                data[field_names[3]] = to_string( (long long) uchar_to_int(buffer, 2)); buffer +=2 ; 
                for (int i=1; i<indx_cnt; i++) {
                    data[field_names[3]] += "," + to_string( (long long) uchar_to_int(buffer, 2)); buffer +=2 ; 
                }
            }
            ADD_TO_FIELD_NAMES
        }
};

class PLR: public BASE {
    public:
        PLR(unsigned char *buffer) {
            rec_name="PLR";
            string field_names[] = { "GRP_CNT", "GRP_INDX", "GRP_MODE", "GRP_RADX", "PGM_CHAR", "RTN_CHAR", 
                                     "PGM_CHAL", "RTN_CHAL" };

            int grp_cnt = uchar_to_int(buffer, 2); buffer += 2; 
            data[field_names[0]] = to_string( (long long) grp_cnt);
            if (grp_cnt > 0) {
                data[field_names[1]] = to_string( (long long) uchar_to_int(buffer, 2)); buffer +=2 ; 
                for (int i=1; i<grp_cnt; i++) {
                    data[field_names[1]] += "," + to_string( (long long) uchar_to_int(buffer, 2)); buffer +=2 ; 
                }

                data[field_names[2]] = to_string( (long long) uchar_to_int(buffer, 2)); buffer +=2 ; 
                for (int i=1; i<grp_cnt; i++) {
                    data[field_names[2]] += "," + to_string( (long long) uchar_to_int(buffer, 2)); buffer +=2 ; 
                }

                data[field_names[3]] = to_string( (long long) *buffer++); 
                for (int i=1; i<grp_cnt; i++) {
                    data[field_names[3]] += "," + to_string( (long long) *buffer++); 
                }
            }
            for (int i=4; i<sizeof(field_names)/sizeof(field_names[0]); i++) {
                data[field_names[i]] = "TODO";
            }
            ADD_TO_FIELD_NAMES
    };
};

class RDR: public BASE {
    public:
        RDR(unsigned char *buffer) {
            rec_name="RDR";
            string field_names[] = { "NUM_BINS", "RTST_BINS" };
            int num_bins = uchar_to_int(buffer, 2); buffer += 2;
            data[field_names[0]] = to_string( (long long) num_bins);
            if (num_bins > 0) {
                data[field_names[1]] = "";
                data[field_names[1]] += to_string( (long long) uchar_to_int(buffer,2)); buffer += 2;
                for (int i=1; i<num_bins; i++) {
                    data[field_names[1]] += "," + to_string( (long long) uchar_to_int(buffer,2)); buffer += 2;
                }
            }
            ADD_TO_FIELD_NAMES 
        };
};

class SDR: public BASE {
    public:
    SDR(unsigned char *buffer) {
        rec_name="SDR";
        string field_names[] = { "HEAD_NUM", "SITE_GRP", "SITE_CNT", "SITE_NUM", "HAND_TYP",
            "HAND_ID", "CARD_TYP", "CARD_ID", "LOAD_TYP", "LOAD_ID", "DIB_TYP", "DIB_ID",
            "CABL_TYP", "CABL_ID", "CONT_TYP", "CONT_ID", "LASR_TYP", "LASR_ID", "EXTR_TYP",
            "EXTR_ID"
            };
        int sites;
        for (int i=0; i<sizeof(field_names)/sizeof(field_names[0]); i++) {
            switch (i) {
                case 0:
                case 1:
                case 2:        
                    sites=*buffer++;
                    data[field_names[i]] = to_string( (long long) sites);
                    break;
                case 3:
                    if (sites > 0) { 
                        data[field_names[i]] = to_string(( long long ) *buffer++);
                
                        for (int j=1; j<sites; j++) {
                            data[field_names[i]] += "," + to_string(( long long ) *buffer++);
                        }
                    }
                    break;
                default:
                    int len=*buffer++;    
                    data[field_names[i]] = "";
                    for (int j=0; j<len; j++) {
                        data[field_names[i]] += *buffer++;
                    }
                    break;
            }
        }
        ADD_TO_FIELD_NAMES
    }
};

class WIR: public BASE {
    public:
        WIR(unsigned char *buffer) {
            rec_name="WIR";
            string field_names[] = { "HEAD_NUM", "SITE_GRP", "START_T", "WAFER_ID" };
            for (int i=0; i<SIZEOF_FIELD_NAMES;i++) {
                switch (i) {
                    case 0:
                    case 1:
                        data[field_names[i]] = to_string( (long long) *buffer++ );
                        break;
                    case 2:
                        data[field_names[i]] = intdate_to_string( uchar_to_int(buffer,4) ); buffer += 4;
                        break;
                    case 3:
                        int len=*buffer++;
                        data[field_names[i]] = "";
                        for (int j=0; j<len; j++) {
                            data[field_names[i]] += *buffer++;
                        }
                }
            }
            ADD_TO_FIELD_NAMES
        };

};

class WRR: public BASE {
    public:
        WRR(unsigned char *buffer) {
            rec_name="WRR";
            string field_names[] = { "HEAD_NUM", "SITE_GRP", "FINISH_T", "PART_CNT", "RTST_CNT",
                "ABRT_CNT", "GOOD_CNT", "FUNC_CNT", "WAFER_ID", "FABWF_ID", "FRAME_ID", "MASK_ID",
                "USR_DESC", "EXC_DESC"
            };

            for (int i=0; i<SIZEOF_FIELD_NAMES; i++) {
                string key=field_names[i];
                switch (i) {
                    case 0:
                    case 1:
                        data[key] = to_string( (long long) *buffer++);
                        break;
                    case 2:
                        data[key] = intdate_to_string( uchar_to_int(buffer,4) ); buffer += 4;
                        break;
                    case 3:
                    case 4:
                    case 5:
                    case 6:
                    case 7:
                        data[key] = to_string( (long long) uchar_to_int(buffer,4) ); buffer += 4;
                        break;
                    default:
                        int len=*buffer++;
                        data[key] = "";
                        for (int j=0; j<len; j++) {
                            data[key] += *buffer++;
                        }
                        break;
                }
            }
            ADD_TO_FIELD_NAMES
        };
};

class WCR: public BASE {
    public:
        WCR(unsigned char *buffer) {
            rec_name="WCR";
            string field_names[] = { "WAFR_SIZ", "DIE_HT", "DIE_WID", "WF_UNITS", "WF_FLAT",
                "CENTER_X", "CENTER_Y", "POS_X", "POS_Y" };    
            for (int i=0; i<SIZEOF_FIELD_NAMES; i++) {
                string key=field_names[i];
                switch (i) {
                    case 0:
                    case 1:
                    case 2:
                        data[key]= to_string( (long long) uchar_to_float(buffer));  buffer += 4; 
                        break;
                    case 3:
                        data[key]= to_string( (long long) *buffer++);  
                        break;
                    case 4:
                    case 7:
                    case 8:
                        data[key] = *buffer++;
                        break; 
                    case 5:
                    case 6:
                        int16_t val = uchar_to_int(buffer, 2); buffer += 2;
                        data[key] = to_string( (long long) val); 
                        break;
                }
            }
            ADD_TO_FIELD_NAMES
        };

};

class PIR: public BASE {
    public:
        PIR(unsigned char *buffer) {
            rec_name="PIR";
            string field_names[] = {"HEAD_NUM", "SITE_NUM" } ;
            for (int i=0; i<SIZEOF_FIELD_NAMES; i++) {
                string key=field_names[i];
                data[key] = to_string( (long long) *buffer++ );
            }
            ADD_TO_FIELD_NAMES
        };
};

class PRR: public BASE {
    public:
        PRR(unsigned char *buffer) {
            rec_name="PRR";
            string field_names[] = {"HEAD_NUM", "SITE_NUM", "PART_FLG", "NUM_TEST", "HARD_BIN", "SOFT_BIN",
                "X_COORD", "Y_COORD", "TEST_T", "PART_ID", "PART_TXT", "PART_FIX"
            };

            for (int i=0; i<SIZEOF_FIELD_NAMES; i++) {
                string key=field_names[i];
                string binstring = "";
                int16_t val=0;
                int len=0;
                switch (i) {
                    case 0:
                    case 1:
                        data[key] = to_string( (long long) *buffer++ );
                        break;
                    case 2:
                        binstring = uchar_to_binary_string(buffer, 1); buffer++;
                        data[key] = binstring;
                        break;
                    case 3:
                    case 4:
                    case 5:
                        data[key] = to_string( (long long) uchar_to_int(buffer,2  )); buffer += 2;
                        break;
                    case 6:
                    case 7:
                        val = uchar_to_int(buffer, 2); buffer += 2;
                        data[key] = to_string( (long long) val); 
                        break;
                    case 8:
                        data[key] = to_string( (long long) uchar_to_int(buffer,4 )); buffer += 4;
                        break;
                    case 9:
                    case 10:
                        len=*buffer++;
                        data[key] = "";
                        for (int j=0; j<len; j++) {
                            data[key] += *buffer++;
                        }
                        break;
                    case 11:
                        len=*buffer++;
                        string binstring = uchar_to_binary_string(buffer, len); buffer += len;
                        data[key] = binstring;
                        break;
                }
            }
            ADD_TO_FIELD_NAMES
        };
};

class TSR: public BASE {
    public:
        TSR(unsigned char *buffer) {
            rec_name="TSR";
            string field_names[] = { "HEAD_NUM", "SITE_NUM", "TEST_TYP", "TEST_NUM", "EXEC_CNT",
                "FAIL_CNT", "ALRM_CNT", "TEST_NAM", "SEQ_NAME", "TEST_LBL", "OPT_FLAG", "TEST_TIM",
                "TEST_MIN", "TEST_MAX", "TST_SUMS", "TST_SQRS"
            }; 


            for (int i=0; i<SIZEOF_FIELD_NAMES; i++) {
                string key=field_names[i], binstring;
                int len;
                ostringstream floatstring;
                floatstring.precision(14);
                switch (i) {
                    case 0:
                    case 1:
                        data[key] = to_string( (long long) *buffer++);
                        break;
                    case 2:
                        data[key] = *buffer++;
                        break;
                    case 3:
                    case 4:
                    case 5:
                    case 6:
                        data[key] = to_string( (long long) uchar_to_int(buffer, 4) ); buffer += 4;
                        break;
                    case 7:
                    case 8:
                    case 9:
                        len = *buffer++;
                        data[key] = "";
                        for (int j=0; j<len; j++) {
                            data[key] += *buffer++;
                        }
                        break;
                    case 10:
                        binstring = uchar_to_binary_string(buffer, 1); buffer++;
                        data[key] = binstring;
                        break;
                    default:
                        floatstring << scientific << uchar_to_float(buffer); buffer += 4;
                        data[key] = floatstring.str(); // to_string( uchar_to_float(buffer)) ; buffer += 4;
                        break ;
                }
            }
            ADD_TO_FIELD_NAMES
        };
};

class PTR: public BASE {
    public:
        PTR(unsigned char *buffer) {
            rec_name="PTR";
            string field_names[] = {"TEST_NUM", "HEAD_NUM", "SITE_NUM", "TEST_FLG", "PARM_FLG", 
                "RESULT", "TEST_TXT", "ALARM_ID", "OPT_FLAG", "RES_SCAL", "LLM_SCAL", "LO_LIMIT", "HI_LIMIT",
                "UNITS", "C_RESFMT", "C_LLMFMT", "C_HLMFMT", "LO_SPEC", "HI_SPEC"
            } ;
            for (int i=0; i<SIZEOF_FIELD_NAMES; i++) {
                string key=field_names[i];
                int len;
                string binstring="";
                ostringstream floatstring;
                floatstring.precision(14);
                switch (i) {
                    case 0:
                        data[key] = to_string(  (long long) uchar_to_int(buffer, 4)); buffer += 4;
                        break;
                    case 1:
                    case 2:
                        data[key] = to_string( (long long) *buffer++);
                        break;
                    case 3:
                    case 4:
                    case 8:
                        binstring = uchar_to_binary_string(buffer, 1); buffer++;
                        data[key] = binstring;
                        break;
                    case 5:
                    case 11:
                    case 12:
                    case 17:
                    case 18:
                        floatstring << scientific << uchar_to_float(buffer); buffer += 4;
                        data[key] = floatstring.str();
                        break;
                    case 6:
                    case 7:
                    case 13:
                    case 14:
                    case 15:
                    case 16:
                        len=*buffer++; 
                        data[key] = "";
                        for (int j=0;j<len; j++) {
                            data[key] += *buffer++;
                        }
                        break;
                    case 9:
                    case 10:
                        data[key] = to_string(  (long long) (int) *buffer++);
                        break;
                }
            }
            ADD_TO_FIELD_NAMES
        };
};

class MPR: public BASE {
    public:
        MPR(unsigned char *buffer) {
            rec_name="MPR";
            string field_names[] = { "TEST_NUM", "HEAD_NUM", "SITE_NUM", "TEST_FLG", "PARM_FLG",
                "RTN_ICNT", "RSLT_CNT", "RTN_STAT", "RTN_RSLT", "TEST_TXT", "ALARM_ID",
                "OPT_FLAG", "RES_SCAL", "LLM_SCAL", "HLM_SCAL", "LO_LIMIT", "HI_LIMIT", "START_IN",
                "INCR_IN", "RTN_INDEX", "UNITS", "UNITS_IN", "C_RESFMT", "C_LLMFMT", "C_HLMFMT", 
                "LO_SPEC", "HI_SPEC"
            };
            int rtn_icnt, rslt_cnt;
            for (int i=0; i<SIZEOF_FIELD_NAMES; i++) {
                string key=field_names[i];
                string binstring="";
                ostringstream floatstring;
                floatstring.precision(14);
                switch(i) {
                    case 0:
                        data[key] = to_string( (long long) uchar_to_int(buffer, 4)); buffer += 4; 
                        break;
                    case 1:
                    case 2:
                        data[key] = to_string( (long long) *buffer++);
                        break;
                    case 3:
                    case 4:
                        binstring = uchar_to_binary_string(buffer, 1); buffer++;
                        data[key] = binstring;
                        break;
                    case 5: 
                        rtn_icnt = uchar_to_int(buffer, 2); buffer += 2;
                        data[key] = to_string( (long long) rtn_icnt); 
                        break;
                    case 6:
                        rslt_cnt = uchar_to_int(buffer, 2); buffer += 2;
                        data[key] = to_string( (long long) rslt_cnt); 
                        break;
                    case 7: {
                        int count=0;
                        data[key] = "";
                        for (int j=0; j<ceil( rtn_icnt/float(2) ); j++) {
                            int rtn_stat = *buffer++;
                            int nh = (rtn_stat >> 4 ) & 0xF;
                            int nl = rtn_stat & 0xF;

                            if (count < rtn_icnt) {
                                if (count == 0) {
                                    data[key] += to_string( (long long) nl);
                                } else {
                                    data[key] += "," + to_string( (long long) nl);
                                }
                                count++;
                            }
                            if (count < rtn_icnt) {
                                data[key] += "," + to_string( (long long) nh);
                                count++;
                            }

                        }
                        break;
                    }
                    case 8:
                        data[key] = "";
                        floatstring.str(""); floatstring.clear();
                        for (int j=0; j<rslt_cnt; j++) {
                            if (j == 0) {
                                floatstring << scientific << uchar_to_float(buffer); buffer += 4;
                            } else { 
                                floatstring << scientific << "," << uchar_to_float(buffer); buffer += 4;
                            }
                        }
                        data[key] += floatstring.str();
                        break;
                    case 20:
                    case 21:
                    case 22:
                    case 23:
                    case 24:
                    case 10:
                    case 9: {
                        int len=*buffer++;
                        data[key] = "";
                        for (int j=0;j<len;j++) {
                            data[key] += *buffer++;
                        }
                        break;
                    }
                    case 11:
                        data[key]= uchar_to_binary_string(buffer++, 1);
                        break;
                    case 12:
                    case 13:
                    case 14:
                        data[key]= to_string( (long long) (int) *buffer++); 
                        break;
                    case 15:
                    case 16:
                    case 17:
                    case 18:
                    case 25:
                    case 26:
                        floatstring.str(""); floatstring.clear();
                        floatstring << scientific << uchar_to_float(buffer); buffer += 4;
                        data[key] = floatstring.str();
                        break;
                    case 19:
                        data[key] = "";
                        for (int j=0; j<rslt_cnt; j++) {
                            if (j==0) {
                                data[key] += to_string ( (long long) uchar_to_int(buffer, 2) ); buffer += 2;
                            } else {
                                data[key] += "," + to_string ( (long long) uchar_to_int(buffer, 2) ); buffer += 2;
                            }
                        }
                        break;
                }
            }
            ADD_TO_FIELD_NAMES
        };
};

class FTR: public BASE {
    public:
        FTR(unsigned char *buffer) {
            rec_name="FTR";
            string field_names[] = { "TEST_NUM", "HEAD_NUM", "SITE_NUM", "TEST_FLG", "OPT_FLG",
                "CYCL_CNT", "REL_VADR", "REPT_CNT", "NUM_FAIL", "XFAIL_AD", "YFAIL_AD", "VECT_OFF",
                "RTN_ICNT", "PGM_ICNT", "RTN_INDX", "RTN_STAT", "PGM_INDX", "PGM_STAT", "FAIL_PIN",
                "VECT_NAM", "TIME_SET", "OP_CODE", "TEST_TXT", "ALARM_ID", "PROG_TXT", "RSLT_TXT", 
                "PATG_NUM", "SPIN_MAP"
            };
            int rtn_icnt, pgm_icnt;
            for (int i=0; i<SIZEOF_FIELD_NAMES; i++) {
                string key=field_names[i];
                switch(i) {
                    case 0:
                    case 5:
                    case 6:
                    case 7:
                    case 8:
                        data[key] = to_string( (long long) uchar_to_int(buffer, 4) ) ; buffer += 4;
                        break;
                    case 1:
                    case 2:
                    case 26:
                        data[key] = to_string ( (long long) *buffer++ ) ;
                        break;
                    case 3:
                    case 4:
                        data[key]= uchar_to_binary_string(buffer++, 1);
                        break;
                    case 9:
                    case 10:
                        data[key] = to_string( (long long) (int) uchar_to_int(buffer, 4) ) ; buffer += 4;
                        break;
                    case 11:
                        data[key] = to_string( (long long) (int16_t) uchar_to_int(buffer, 2) ) ; buffer += 2;
                        break;
                    case 12:
                        rtn_icnt = (uint16_t) uchar_to_int(buffer, 2);
                        data[key] = to_string( (long long) (uint16_t) uchar_to_int(buffer, 2) ) ; buffer += 2;
                        break;
                    case 13:
                        pgm_icnt = (uint16_t) uchar_to_int(buffer, 2);
                        data[key] = to_string( (long long) (uint16_t) uchar_to_int(buffer, 2) ) ; buffer += 2;
                        break;
                    case 16:
                    case 14:{
                        data[key] = "";
                        int counter=(i==14)?rtn_icnt:pgm_icnt;
                        for (int j=0;j<counter; j++) {
                            if (j==0) {
                                data[key] = to_string( (long long) (uint16_t) uchar_to_int(buffer, 2) );buffer += 2;
                            } else {
                                data[key] += "," +  to_string( (long long) (uint16_t) uchar_to_int(buffer, 2) );buffer += 2;
                            }
                        }
                        break;
                    }
                    case 17:
                    case 15: {
                        int count=0;
                        data[key] = "";
                        int counter=(i==15)?rtn_icnt:pgm_icnt;
                        for (int j=0; j<ceil( counter/float(2) ); j++) {
                            int rtn_stat = *buffer++;
                            int nh = (rtn_stat >> 4 ) & 0xF;
                            int nl = rtn_stat & 0xF;

                            if (count < rtn_icnt) {
                                if (count == 0) {
                                    data[key] += to_string( (long long) nl);
                                } else {
                                    data[key] += "," + to_string( (long long) nl);
                                }
                                count++;
                            }
                            if (count < rtn_icnt) {
                                data[key] += "," + to_string( (long long) nh);
                                count++;
                            }

                        }
                        break;
                    }
                    case 27:
                    case 18: {
                        int len = uchar_to_int(buffer, 2); buffer += 2; 
                        int bytes = ceil( len/float(8) );   
                        string binstring = uchar_to_binary_string(buffer, bytes); buffer += bytes;
                        data[key]= binstring.substr( (bytes*8)-len );
                        break;                    
                    }
                    default: {
                        int len=*buffer++;
                        data[key]= "";
                        for (int j=0;j<len; j++) {
                            data[key] += *buffer++;
                        }
                        break;
                    }           
                }
            }
            ADD_TO_FIELD_NAMES
        };
};

class BPS: public BASE {
    public:
        BPS(unsigned char *buffer) {
            rec_name="BPS";
            string field_names[] = { "SEQ_NAME" };
            data["SEQ_NAME"] = "";
            int len=*buffer++;
            for (int j=0; j<len; j++) {
                data["SEQ_NAME"] += *buffer++;
            }
            ADD_TO_FIELD_NAMES
        };
};

class EPS: public BASE {
    public:
        EPS(unsigned char *buffer) {
            rec_name="EPS";

        };
};

class GDR: public BASE {
    public:
        GDR(unsigned char *buffer) {
            rec_name="GDR";
            string field_names[] = { "FLD_CNT", "GEN_DATA" } ;
            int fld_cnt;
            for (int i=0;i<SIZEOF_FIELD_NAMES; i++) {
                string key=field_names[i];
                switch (i) {
                    case 0:
                        fld_cnt = uchar_to_int(buffer, 2); buffer += 2;
                        data[key]= to_string( (long long) fld_cnt );
                        break;
                    case 1: {
                        data[key]= "";
                        for (int j=0;j<fld_cnt; j++) {
                            data[key] += "\n";
                            int data_type = *buffer++;
                            switch (data_type) {
                                case 0:
                                    //buffer++;
                                    break;
                                case 1:
                                    data[key] += to_string ( (long long) (uint8_t) *buffer++ ) ;
                                    break;
                                case 2:
                                    data[key] += to_string ( (long long) (uint16_t) uchar_to_int(buffer, 2) ); buffer += 2;
                                    break;
                                case 3:
                                    data[key] += to_string ( (long long) (uint32_t) uchar_to_int(buffer, 4) ); buffer += 4;
                                    break;
                                case 4:
                                    data[key] += to_string ( (long long) (int8_t) *buffer++ ) ;
                                    break;
                                case 5:
                                    data[key] += to_string ( (long long) (int16_t) uchar_to_int(buffer, 2) ); buffer += 2;
                                    break;
                                case 6:
                                    data[key] += to_string ( (long long) (int32_t) uchar_to_int(buffer, 4) ); buffer += 4;
                                    break;
                                case 7:
                                    data[key] += to_string ( (long long) uchar_to_float(buffer) ); buffer += 4;
                                    break;
                                case 8:
                                    data[key] += to_string ( (long long) uchar_to_float64(buffer) ); buffer += 8;
                                    break;
                                case 10: {
                                    int len=*buffer++;
                                    for (int j=0; j<len; j++) {
                                        data[key] += *buffer++;
                                    }
                                    break;
                                }
                                case 11: {
                                    int len = *buffer++;
                                    data[key] += uchar_to_binary_string(buffer, len); buffer += len;
                                    break;
                                }
                                case 12: {
                                    int len = uchar_to_int(buffer, 2); buffer += 2; 
                                    int bytes = ceil( len/float(8) );   
                                    string binstring = uchar_to_binary_string(buffer, bytes); buffer += bytes;
                                    data[key] += binstring.substr( (bytes*8)-len );
                                    break;
                                }
                                case 13:
                                    data[key] += to_string ( (long long) *buffer & 0xF ); buffer++; 
                                    break; 
                            }
                        }
                        break;
                    }
                }
            }
            ADD_TO_FIELD_NAMES
        };
};

class DTR: public BASE {
    public:
        DTR(unsigned char *buffer) {
            rec_name="DTR";
            string field_names[] = { "TEXT_DAT" };
            int len=*buffer++;
            data["TEXT_DAT"] = "";
            for (int j=0; j<len; j++) {
                data["TEXT_DAT"] += *buffer;
            }
            ADD_TO_FIELD_NAMES
        };

};


class Stdf_parser {
    private:
        string stdf_filename;        
        ifstream stdf_fh;
        int stdf_parser_state;
        int file_len;
        int record_count;

        void stdf_parser_init();
        vector<header_t> headers;

        int read_header(header_t& header);
        void read_data(const header_t& header,char *buffer);
        char *allocate_data_buffer(const header_t& header);
        BASE decode_stdf_record(const header_t& header);
    public:
        Stdf_parser(string stdf_filename):stdf_filename(stdf_filename), stdf_parser_state(1), record_count(0) { stdf_parser_init(); } ;
        Stdf_parser():stdf_parser_state(1), record_count(0) {} ;
        ~Stdf_parser();

        void stdf_parser_init(string stdf_filename) {
            this->stdf_filename = stdf_filename;
            this->stdf_parser_state = 1;
            this->record_count = 0;
            stdf_parser_init();
        };

        vector<header_t> get_all_headers() { return headers;};
        int get_stdf_parser_state() { return stdf_parser_state; };
        //print the entire stdf
        void stdf_as_text();
        //add a vector to hold the strings, so that the results can be used by other functions.
        //this is to make sure that only 1 record is saved. This is to not blow up the memory.
        void record_as_text(const header_t& header, vector<string>& records, bool quiet=false);
        void records_as_text(const vector<header_t>&  filtered_headers);
        vector<header_t> get_stdf_headers_of_type(string rec_names, int rec_id_start=0, int rec_id_end=0);
        int get_record_count() { return record_count; };
        string get_rec_name(const header_t& header);

        
};


#endif
