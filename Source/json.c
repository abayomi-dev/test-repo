#include "global.h"

int chkBiller = 0;

static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
	if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start &&
			strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
		return 0;
	}
	return -1;
}

//Use configinitapplicationversion to check for new updates
int parseProfile(char *data) {
	int i;
	int r;
	jsmn_parser p;
	jsmntok_t t[300]; /* We expect no more than 128 tokens */
	char storeData[100 * 1024] = {0};
	char chckpin[10] = {0};

	ShowLogs(1, "1. Length of Data: %d", strlen(data));

	jsmn_init(&p);
	r = jsmn_parse(&p, data, strlen(data), t, sizeof(t)/sizeof(t[0]));
	if (r < 0) {
		if(checkNew)
		{
			remove("profile.txt");
			//Monitor this
		}
		//printf("Failed to parse JSON: %d\n", r);
		ShowLogs(1, "Failed to parse JSON: %d", r);
		ShowLogs(1, "2. Length of Data: %d", strlen(data));
		return 1;
	}

	ShowLogs(1, "Data Gotten: %d", strlen(data));

	//ShowLogs(1, "R returned: %d, Type: %d", r, t[0].type);

	/* Assume the top-level element is an object */
	if (r < 1 || t[0].type != JSMN_OBJECT) {
		//printf("Object expected\n");
		ShowLogs(1, "Object expected.");
		return 1;
	}

	memset(&profileTag, 0, sizeof(PROFILETAG));
	//sprintf((char *)profileTag.vervehost, "%s", "0200");
	/* Loop over all keys of the root object */
	for (i = 1; i < r; i++) {
		if (jsoneq(data, &t[i], "timestamp") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("tmDt", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "tid") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("tid", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "actsel") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("actsels", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "mid") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("mid", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "serialnumber") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("serialnumber", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "terminalmodel") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("terminalmodel", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "initapplicationversion") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("initapplicationversion", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "merchantname") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("merName", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "merchantaddress") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("merAddr", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "adminpin") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("adminpin", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "merchantpin") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("merchantpin", storeData);
			if(filesize("merchantpin.txt") < 4)
			{
				CreateWrite("merchantpin.txt", storeData);
			}
			i++;
		} else if (jsoneq(data, &t[i], "changepin") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("changepin", storeData);
			if(strcmp(storeData, "true") == 0)
			{
				char pin[10] = {0};
				memset(pin, '\0', strlen(pin));
				UtilGetEnvEx("merchantpin", pin);
				CreateWrite("merchantpin.txt", pin);
			}
			i++;
		} else if (jsoneq(data, &t[i], "contactname") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("contactname", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "contactphone") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("contactphone", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "configstampduty") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("stampduty", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "email") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("email", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "mcc") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("mcc", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "lga") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("lga", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "appname") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("appname", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "country") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("country", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "countrycode") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("countrycode", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "profilename") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("profilename", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "blocked") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("blocked", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "blockedpin") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("blockedpin", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "ownerusername") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("ownerusername", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "superagent") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("superagent", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "dialogheading") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("dialogheading", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "simserial") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("simserial", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "simnumber") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("simnumber", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "simname") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("simname", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "accountname") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("accountname", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "accountcode") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("accountcode", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "accountnumber") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("accountnumber", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "accountbank") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("accountbank", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "profiletransactiontypesarray") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("profiletransactiontypesarray", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "profilecardschemekeytypes") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("profilecardschemekeytypes", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "profileprotectlist") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("profileprotectlist", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "profilevas") == 0) {
			int val = 0;
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("profilevas", storeData);
			chkBiller = 0;
			val = atoi(storeData);
			if(val)
			{
				chkBiller = val;
			}
		 	else
		 	{
		 		remove("bMenu.txt");
		 	}
			i++;
		} else if (jsoneq(data, &t[i], "profilehostarray") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("profilehostarray", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "profilekarrabopay") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("profilekarrabopay", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "chname") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("chname", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "chremotedownloadtime") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("rmDwnTime", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "chinterval") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("chinterval", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "chip") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("chip", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "chport") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("chport", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "chcount") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("chremarks", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "comname") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("coname", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "comcommstype") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("cotype", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "comusername") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("cosubnet", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "comgateway") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("cogateway", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "comip") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("coip", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "comport") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("coport", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "comipmode") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("coipmode", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "comapn") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("coapn", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "compassword") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("copwd", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "comremarks") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("coremarks", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "rptname") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("rptname", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "rptfootertext") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("rptfootertext", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "rptcustomercopylabel") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("rptcclabel", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "rptmerchantcopylabel") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("rptmclabel", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "rptfootnotelabel") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("rptfnlabel", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "rptshowlogo") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			//UtilPutEnv("rptshowlogo", storeData);
			UtilPutEnv("rptshowlogo", "true");
			i++;
		} else if (jsoneq(data, &t[i], "rptnormalfontsize") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("rptnfsize", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "rptheaderfontsize") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("rpthfsize", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "rptamountfontsize") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("rptamtsize", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "rptshowbarcode") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("rptsbarcode", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "rptprintmerchantcopynumber") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("rptpmcn", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "rptprintclientcopynumber") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("rptpccn", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "rptsaveforreceipt") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("rptsfrpnt", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "rptheadervalue") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("rptheadervalue", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "hostidname") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("hostname", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "hostip") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("hostip",storeData );//storeData
			UtilPutEnv("uhostip", storeData);//storeData
			i++;
		} else if (jsoneq(data, &t[i], "hostport") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("hostport",storeData );//
			UtilPutEnv("uhostport", storeData);//storeData
			i++;
		} else if (jsoneq(data, &t[i], "hostssl") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("hostssl", storeData);
			UtilPutEnv("uhostssl", storeData);
			if(strcmp(storeData,"false")==0){
				PutEnv("E_SSL", "0");					
			}else{
				PutEnv("E_SSL", "1");
			}
			i++;
		} else if (jsoneq(data, &t[i], "hostfriendlyname") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("hostfname", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "hostmestype") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("hostmestype", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "hostid2name") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("host2name", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "host2ip") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("host2ip", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "host2port") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("host2port", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "host2ssl") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("host2ssl", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "host2friendlyname") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("host2fname", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "host2mestype") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("host2mestype", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "swkname") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("swkname", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "swkcomponent1") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("swkcomp1", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "swkcomponent2") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("swkcomp2", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "swk2name") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("swkfname", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "swk2component1") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("swkfcomp1", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "swk2component2") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("swkfcomp2", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "curname") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("curname", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "curabbreviation") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("curabbr", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "curcode") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("curcode", storeData);
			UtilPutEnv("txnCurCode", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "curminorunit") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("curminorunit", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "curremarks") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("curremarks", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "bnkname") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("bankname", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "bnkcode") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("bankcode", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "bnkremarks") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("bankremarks", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "transactions") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			if(strstr(storeData, "Logout") == NULL)
		    {
		        CreateWrite("nocashier.txt", "NOCASHIERPLEASE");
		    }else
		    {
		    	remove("nocashier.txt");
		    }
			CreateWrite("menus.txt", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "hostarray") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			CreateWrite("host.txt", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "protectlist") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			CreateWrite("protectlist.txt", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "appversion") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("appversion", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "appbrand") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("appbrand", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "appdescription") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("appdesc", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "appmodel") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("appmodel", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "appfix") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("appfix", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "appupdated") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("appupdated", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "appremarks") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("appremarks", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "logorversion") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("logoversion", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "logorfilename") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("logofilename", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "logordescription") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("logodesc", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "logorbankcode") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			sprintf(profileTag.bankcode, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("logobankcode", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "logorbankname") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("logobankname", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "logobversion") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("logobversion", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "logobfilename") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("logobfilename", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "logobdescription") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("logobdesc", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "logobbankcode") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("logobbankcode", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "logobbankname") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("logobbankname", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "paymentdetails") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			CreateWrite("payment.txt", storeData);
			i++;
		} else {
			//ShowLogs(1, "Unexpected key: %.*s\n", t[i].end-t[i].start, data + t[i].start);
		}
	}
	return 0;
}

int parseProfileOld(char *data) {
	int i;
	int r;
	jsmn_parser p;
	jsmntok_t t[300]; /* We expect no more than 128 tokens */
	char storeData[10 * 1024] = {0};

	jsmn_init(&p);
	r = jsmn_parse(&p, data, strlen(data), t, sizeof(t)/sizeof(t[0]));
	if (r < 0) {
		//printf("Failed to parse JSON: %d\n", r);
		//ShowLogs(1, "Failed to parse JSON: %d", r);
		return 1;
	}

	//ShowLogs(1, "R returned: %d, Type: %d", r, t[0].type);

	/* Assume the top-level element is an object */
	if (r < 1 || t[0].type != JSMN_OBJECT) {
		//printf("Object expected\n");
		//ShowLogs(1, "Object expected.");
		return 1;
	}
	memset(&profileTag, 0, sizeof(PROFILETAG));
	/* Loop over all keys of the root object */
	for (i = 1; i < r; i++) {
		if (jsoneq(data, &t[i], "timestamp") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			//UtilPutEnv("tmDt", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "tid") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("tid", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "actsel") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("actsels", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "mid") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("mid", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "serialnumber") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("serialnumber", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "terminalmodel") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("terminalmodel", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "initapplicationversion") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("initapplicationversion", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "merchantname") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("merName", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "merchantaddress") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("merAddr", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "adminpin") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("adminpin", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "merchantpin") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("merchantpin", storeData);
			if(filesize("merchantpin.txt") < 4)
			{
				CreateWrite("merchantpin.txt", storeData);
			}
			i++;
		} else if (jsoneq(data, &t[i], "changepin") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("changepin", storeData);
			if(strcmp(storeData, "true") == 0)
			{
				char pin[10] = {0};
				memset(pin, '\0', strlen(pin));
				UtilGetEnvEx("merchantpin", pin);
				CreateWrite("merchantpin.txt", pin);
			}
			i++;
		} else if (jsoneq(data, &t[i], "contactname") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("contactname", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "contactphone") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("contactphone", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "configstampduty") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("stampduty", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "email") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("email", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "mcc") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("mcc", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "lga") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("lga", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "appname") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("appname", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "country") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("country", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "countrycode") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("countrycode", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "profilename") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("profilename", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "blocked") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("blocked", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "blockedpin") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("blockedpin", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "ownerusername") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("ownerusername", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "superagent") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("superagent", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "dialogheading") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("dialogheading", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "simserial") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("simserial", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "simnumber") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("simnumber", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "simname") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("simname", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "accountname") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("accountname", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "accountcode") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("accountcode", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "accountnumber") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("accountnumber", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "accountbank") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("accountbank", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "profiletransactiontypesarray") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("profiletransactiontypesarray", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "profilecardschemekeytypes") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("profilecardschemekeytypes", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "profileprotectlist") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("profileprotectlist", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "profilevas") == 0) {
			int val = 0;
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("profilevas", storeData);
			chkBiller = 0;
			val = atoi(storeData);
			if(val)
			{
				chkBiller = val;
			}
		 	else
		 	{
		 		remove("bMenu.txt");
		 	}
			i++;
		} else if (jsoneq(data, &t[i], "profilehostarray") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("profilehostarray", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "profilekarrabopay") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("profilekarrabopay", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "chname") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("chname", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "chremotedownloadtime") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("rmDwnTime", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "chinterval") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("chinterval", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "chip") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("chip", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "chport") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("chport", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "chcount") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("chremarks", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "comname") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("coname", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "comcommstype") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("cotype", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "comusername") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("cosubnet", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "comgateway") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("cogateway", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "comip") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("coip", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "comport") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("coport", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "comipmode") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("coipmode", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "comapn") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("coapn", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "compassword") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("copwd", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "comremarks") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("coremarks", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "rptname") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("rptname", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "rptfootertext") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("rptfootertext", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "rptcustomercopylabel") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("rptcclabel", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "rptmerchantcopylabel") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("rptmclabel", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "rptfootnotelabel") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("rptfnlabel", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "rptshowlogo") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			//UtilPutEnv("rptshowlogo", storeData);
			UtilPutEnv("rptshowlogo", "true");
			i++;
		} else if (jsoneq(data, &t[i], "rptnormalfontsize") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("rptnfsize", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "rptheaderfontsize") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("rpthfsize", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "rptamountfontsize") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("rptamtsize", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "rptshowbarcode") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("rptsbarcode", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "rptprintmerchantcopynumber") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("rptpmcn", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "rptprintclientcopynumber") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("rptpccn", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "rptsaveforreceipt") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("rptsfrpnt", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "rptheadervalue") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("rptheadervalue", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "hostidname") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("hostname", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "hostip") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("hostip", storeData);//
			UtilPutEnv("uhostip", storeData);//
			i++;
		} else if (jsoneq(data, &t[i], "hostport") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("hostport", storeData);//storeData
			UtilPutEnv("uhostport", storeData);//storeData
			i++;
		} else if (jsoneq(data, &t[i], "hostssl") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("hostssl", storeData);
			UtilPutEnv("uhostssl", storeData);
			if(strcmp(storeData,"false")==0){
				PutEnv("E_SSL", "0");					
			}else{
				PutEnv("E_SSL", "1");
			}
			i++;
		} else if (jsoneq(data, &t[i], "hostfriendlyname") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("hostfname", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "hostmestype") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("hostmestype", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "hostid2name") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("host2name", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "host2ip") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("host2ip", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "host2port") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("host2port", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "host2ssl") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("host2ssl", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "host2friendlyname") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("host2fname", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "host2mestype") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("host2mestype", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "swkname") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("swkname", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "swkcomponent1") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("swkcomp1", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "swkcomponent2") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("swkcomp2", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "swk2name") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("swkfname", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "swk2component1") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("swkfcomp1", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "swk2component2") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("swkfcomp2", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "curname") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("curname", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "curabbreviation") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("curabbr", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "curcode") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("curcode", storeData);
			UtilPutEnv("txnCurCode", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "curminorunit") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("curminorunit", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "curremarks") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("curremarks", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "bankname") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("bankname", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "bankcode") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("bankcode", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "bankremarks") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("bankremarks", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "transactions") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			if(strstr(storeData, "Logout") == NULL)
		    {
		        CreateWrite("nocashier.txt", "NOCASHIERPLEASE");
		    }else
		    {
		    	remove("nocashier.txt");
		    }
			CreateWrite("menus.txt", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "hostarray") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			CreateWrite("host.txt", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "protectlist") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			CreateWrite("protectlist.txt", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "appversion") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("appversion", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "appbrand") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("appbrand", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "appdescription") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("appdesc", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "appmodel") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("appmodel", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "appfix") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("appfix", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "appupdated") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("appupdated", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "appremarks") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("appremarks", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "logorversion") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("logoversion", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "logorfilename") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("logofilename", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "logordescription") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("logodesc", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "logorbankcode") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			sprintf(profileTag.bankcode, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("logobankcode", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "logorbankname") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("logobankname", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "logobversion") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("logobversion", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "logobfilename") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("logobfilename", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "logobdescription") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("logobdesc", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "logobbankcode") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("logobbankcode", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "logobbankname") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			UtilPutEnv("logobbankname", storeData);
			i++;
		} else if (jsoneq(data, &t[i], "paymentdetails") == 0) {
			memset(storeData, '\0', strlen(storeData));
			sprintf(storeData, "%.*s", t[i+1].end-t[i+1].start, data + t[i+1].start);
			CreateWrite("payment.txt", storeData);
			i++;
		} else {
			//ShowLogs(1, "Unexpected key: %.*s\n", t[i].end-t[i].start, data + t[i].start);
		}
	}
	//UtilPutEnv("tid", "2UP10255 ");//Delete.
	return 0;
}
