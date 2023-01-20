#include "jsmn.h"
extern char Cel_responseCode[20];
extern char Cel_responseMessage[70];
extern char Cel_BenAmunt[22];
extern char Cel_TransactionId[22];
extern char Cel_AccountName[22];
extern char Cel_TimeStamp[22];
extern char Cel_PlatformID[10];
extern char Cel_Token[90];
extern char Cel_Fname[22];
extern char Cel_Lname[10];
extern char Cel_AgentID[10];

//eedc variables
extern char eedc_transaction_reference[20];
extern char eedc_units[20];
extern char eedc_appliedToArrears[20];
extern char eedc_token[90];
extern char eedc_phone [10];
extern char eedc_vat[6];
extern char eedc_customerName[40];
extern char eedc_convenience[10];
extern char eedc_total [10];
//END EEDC 

//jamb
extern char jamb_customerName[40];
extern int JAMB_CODE[15];
extern int EEDC_CUS_ID[15];


//end jamb

/**
 * Allocates a fresh unused token from the token pool.
 */

static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
	if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start &&
			strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
		return 0;
	}
	return -1;
}


static jsmntok_t *jsmn_alloc_token(jsmn_parser *parser,
		jsmntok_t *tokens, size_t num_tokens) {
	jsmntok_t *tok;
	if (parser->toknext >= num_tokens) {
		return NULL;
	}
	tok = &tokens[parser->toknext++];
	tok->start = tok->end = -1;
	tok->size = 0;
#ifdef JSMN_PARENT_LINKS
	tok->parent = -1;
#endif
	return tok;
}

/**
 * Fills token type and boundaries.
 */
static void jsmn_fill_token(jsmntok_t *token, jsmntype_t type,
                            int start, int end) {
	token->type = type;
	token->start = start;
	token->end = end;
	token->size = 0;
}

/**
 * Fills next available token with JSON primitive.
 */
static int jsmn_parse_primitive(jsmn_parser *parser, const char *js,
		size_t len, jsmntok_t *tokens, size_t num_tokens) {
	jsmntok_t *token;
	int start;

	start = parser->pos;

	for (; parser->pos < len && js[parser->pos] != '\0'; parser->pos++) {
		switch (js[parser->pos]) {
#ifndef JSMN_STRICT
			/* In strict mode primitive must be followed by "," or "}" or "]" */
			case ':':
#endif
			case '\t' : case '\r' : case '\n' : case ' ' :
			case ','  : case ']'  : case '}' :
				goto found;
		}
		if (js[parser->pos] < 32 || js[parser->pos] >= 127) {
			parser->pos = start;
			return JSMN_ERROR_INVAL;
		}
	}
#ifdef JSMN_STRICT
	/* In strict mode primitive must be followed by a comma/object/array */
	parser->pos = start;
	return JSMN_ERROR_PART;
#endif

found:
	if (tokens == NULL) {
		parser->pos--;
		return 0;
	}
	token = jsmn_alloc_token(parser, tokens, num_tokens);
	if (token == NULL) {
		parser->pos = start;
		ShowLogs(1, "A. Checking Error Step 0");
		return JSMN_ERROR_NOMEM;
	}
	jsmn_fill_token(token, JSMN_PRIMITIVE, start, parser->pos);
#ifdef JSMN_PARENT_LINKS
	token->parent = parser->toksuper;
#endif
	parser->pos--;
	return 0;
}

/**
 * Fills next token with JSON string.
 */
static int jsmn_parse_string(jsmn_parser *parser, const char *js,
		size_t len, jsmntok_t *tokens, size_t num_tokens) {
	jsmntok_t *token;

	int start = parser->pos;

	parser->pos++;

	/* Skip starting quote */
	for (; parser->pos < len && js[parser->pos] != '\0'; parser->pos++) {
		char c = js[parser->pos];

		/* Quote: end of string */
		if (c == '\"') {
			if (tokens == NULL) {
				return 0;
			}
			token = jsmn_alloc_token(parser, tokens, num_tokens);
			if (token == NULL) {
				parser->pos = start;
				ShowLogs(1, "B. Checking Error Step B");
				return JSMN_ERROR_NOMEM;
			}
			jsmn_fill_token(token, JSMN_STRING, start+1, parser->pos);
#ifdef JSMN_PARENT_LINKS
			token->parent = parser->toksuper;
#endif
			return 0;
		}

		/* Backslash: Quoted symbol expected */
		if (c == '\\' && parser->pos + 1 < len) {
			int i;
			parser->pos++;
			switch (js[parser->pos]) {
				/* Allowed escaped symbols */
				case '\"': case '/' : case '\\' : case 'b' :
				case 'f' : case 'r' : case 'n'  : case 't' :
					break;
				/* Allows escaped symbol \uXXXX */
				case 'u':
					parser->pos++;
					for(i = 0; i < 4 && parser->pos < len && js[parser->pos] != '\0'; i++) {
						/* If it isn't a hex character we have an error */
						if(!((js[parser->pos] >= 48 && js[parser->pos] <= 57) || /* 0-9 */
									(js[parser->pos] >= 65 && js[parser->pos] <= 70) || /* A-F */
									(js[parser->pos] >= 97 && js[parser->pos] <= 102))) { /* a-f */
							parser->pos = start;
							return JSMN_ERROR_INVAL;
						}
						parser->pos++;
					}
					parser->pos--;
					break;
				/* Unexpected symbol */
				default:
					parser->pos = start;
					return JSMN_ERROR_INVAL;
			}
		}
	}
	parser->pos = start;
	return JSMN_ERROR_PART;
}

/**
 * Parse JSON string and fill tokens.
 */
int jsmn_parse(jsmn_parser *parser, const char *js, size_t len,
		jsmntok_t *tokens, unsigned int num_tokens) {
	int r;
	int i;
	jsmntok_t *token;
	int count = parser->toknext;

	for (; parser->pos < len && js[parser->pos] != '\0'; parser->pos++) {
		char c;
		jsmntype_t type;

		c = js[parser->pos];
		switch (c) {
			case '{': case '[':
				count++;
				if (tokens == NULL) {
					break;
				}
				token = jsmn_alloc_token(parser, tokens, num_tokens);
				ShowLogs(1, "Checking Error Step 0");
				if (token == NULL)
					return JSMN_ERROR_NOMEM;
				ShowLogs(1, "Checking Error Step 1");
				if (parser->toksuper != -1) {
					tokens[parser->toksuper].size++;
#ifdef JSMN_PARENT_LINKS
					token->parent = parser->toksuper;
#endif
				}
				token->type = (c == '{' ? JSMN_OBJECT : JSMN_ARRAY);
				token->start = parser->pos;
				parser->toksuper = parser->toknext - 1;
				break;
			case '}': case ']':
				if (tokens == NULL)
					break;
				type = (c == '}' ? JSMN_OBJECT : JSMN_ARRAY);
#ifdef JSMN_PARENT_LINKS
				if (parser->toknext < 1) {
					return JSMN_ERROR_INVAL;
				}
				token = &tokens[parser->toknext - 1];
				for (;;) {
					if (token->start != -1 && token->end == -1) {
						if (token->type != type) {
							return JSMN_ERROR_INVAL;
						}
						token->end = parser->pos + 1;
						parser->toksuper = token->parent;
						break;
					}
					if (token->parent == -1) {
						if(token->type != type || parser->toksuper == -1) {
							return JSMN_ERROR_INVAL;
						}
						break;
					}
					token = &tokens[token->parent];
				}
#else
				for (i = parser->toknext - 1; i >= 0; i--) {
					token = &tokens[i];
					if (token->start != -1 && token->end == -1) {
						if (token->type != type) {
							return JSMN_ERROR_INVAL;
						}
						parser->toksuper = -1;
						token->end = parser->pos + 1;
						break;
					}
				}
				/* Error if unmatched closing bracket */
				if (i == -1) return JSMN_ERROR_INVAL;
				for (; i >= 0; i--) {
					token = &tokens[i];
					if (token->start != -1 && token->end == -1) {
						parser->toksuper = i;
						break;
					}
				}
#endif
				break;
			case '\"':
				r = jsmn_parse_string(parser, js, len, tokens, num_tokens);
				if (r < 0) return r;
				count++;
				if (parser->toksuper != -1 && tokens != NULL)
					tokens[parser->toksuper].size++;
				break;
			case '\t' : case '\r' : case '\n' : case ' ':
				break;
			case ':':
				parser->toksuper = parser->toknext - 1;
				break;
			case ',':
				if (tokens != NULL && parser->toksuper != -1 &&
						tokens[parser->toksuper].type != JSMN_ARRAY &&
						tokens[parser->toksuper].type != JSMN_OBJECT) {
#ifdef JSMN_PARENT_LINKS
					parser->toksuper = tokens[parser->toksuper].parent;
#else
					for (i = parser->toknext - 1; i >= 0; i--) {
						if (tokens[i].type == JSMN_ARRAY || tokens[i].type == JSMN_OBJECT) {
							if (tokens[i].start != -1 && tokens[i].end == -1) {
								parser->toksuper = i;
								break;
							}
						}
					}
#endif
				}
				break;
#ifdef JSMN_STRICT
			/* In strict mode primitives are: numbers and booleans */
			case '-': case '0': case '1' : case '2': case '3' : case '4':
			case '5': case '6': case '7' : case '8': case '9':
			case 't': case 'f': case 'n' :
				/* And they must not be keys of the object */
				if (tokens != NULL && parser->toksuper != -1) {
					jsmntok_t *t = &tokens[parser->toksuper];
					if (t->type == JSMN_OBJECT ||
							(t->type == JSMN_STRING && t->size != 0)) {
						return JSMN_ERROR_INVAL;
					}
				}
#else
			/* In non-strict mode every unquoted value is a primitive */
			default:
#endif
				r = jsmn_parse_primitive(parser, js, len, tokens, num_tokens);
				if (r < 0) return r;
				count++;
				if (parser->toksuper != -1 && tokens != NULL)
					tokens[parser->toksuper].size++;
				break;

#ifdef JSMN_STRICT
			/* Unexpected char in strict mode */
			default:
				return JSMN_ERROR_INVAL;
#endif
		}
	}

	if (tokens != NULL) {
		for (i = parser->toknext - 1; i >= 0; i--) {
			/* Unmatched opened object or array */
			if (tokens[i].start != -1 && tokens[i].end == -1) {
				return JSMN_ERROR_PART;
			}
		}
	}

	return count;
}

/**
 * Creates a new parser based over a given  buffer with an array of tokens
 * available.
 */
void jsmn_init(jsmn_parser *parser) {
	parser->pos = 0;
	parser->toknext = 0;
	parser->toksuper = -1;
}



int ParseJson(char * JSON_STRING) {
	int i;
	int r;char tempvar[50];char BUFF[50];
	int nbBankCode=0;int nbBankName=0;
	int nb_1=0, nb_2=0, nb_3=0,nb_4=0,nb_5=0,nb_6=0,nb_7=0,nb_8=0;

	jsmn_parser p;
	jsmntok_t t[1000]; /* 128 We expect no more than 128 tokens */
	char jbuffer[400]={0};//300
	int nbTVAmt=0;int nbTVProd=0;

	jsmn_init(&p);
	r = jsmn_parse(&p, JSON_STRING, strlen(JSON_STRING), t, sizeof(t)/sizeof(t[0]));
	if (r < 0) {
		//printf("Failed to parse JSON: %d\n", r);
		memset(BUFF,0x00,sizeof(BUFF));
		sprintf(BUFF,"Failed to parse JSON: %d\n", r);
		displayMessageWt("ParseJson",BUFF);
		//displa("WARNING!!!",BUFF);//getkeys();
		return 1;
	}

	/* Assume the top-level element is an object */
	if (r < 1 || t[0].type != JSMN_OBJECT) {
		printf("Object expected\n");
		displayMessageWt("ParseJson","Object expected");
		return 1;
	}
	
	/* Loop over all keys of the root object */
	nbBankCode=0;
	nbBankName=0;
	for (i = 1; i < r; i++) {
	//	switch(function){
	//	case TRAN_TYPE_TMS_NOTIFICATION:

		//{"ResponseCode":"07","ResponseDescription":"ACCOUNT INVALID","URL":"http:\/\/urlLoading"},
//Received:{"status":"failed","message":"Validation Errors","errors":{"email":["The email field is required."]}}
//{"status":"success","message":"User Logged In","token":"eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJfaWQiOiI2MTFhMjU4YTQ3Yjc2NjFhOWE2MzBiMmUiLCJmaXJzdE5hbWUiOiJTaGFudSIsImxhc3ROYW1lIjoiT2x1d2FzZWd1biIsImVtYWlsIjoic2hhbnVvbHV3YXNlZ3VuQGdtYWlsLmNvbSIsInBob25lTnVtYmVyIjoiMDcwNjM1MDQ1MDkiLCJ1c2VyVHlwZSI6ImFnZW50IiwiYWdlbnRJZCI6IjcwMDYwMjMxMyIsImlhdCI6MTY0MTk5ODU3MSwiZXhwIjoxNjQyMDA1NzcxfQ.XW0WgewlaF_ytGEAskjW86toLyJkqInLgSJQCk1Sq98","expiresIn":"2 Hours",

//"userData":{"_id":"611a258a47b7661a9a630b2e","firstName":"Shanu","lastName":"Oluwasegun","email":"shanuoluwasegun@gmail.com","phoneNumber":"07063504509","userType":"agent","agentId":"700602313"}},
			if (jsoneq(JSON_STRING, &t[i], "status") == 0) {
				memset(Cel_responseCode, 0, sizeof(Cel_responseCode));
				sprintf(Cel_responseCode,"%.*s", t[i+1].end-t[i+1].start,JSON_STRING + t[i+1].start);
				//displayMessageWt("ResponseCode",Cel_responseCode);
				i++;
			}else if (jsoneq(JSON_STRING, &t[i], "message") == 0) {
				memset(Cel_responseMessage, 0, sizeof(Cel_responseMessage));
				sprintf(Cel_responseMessage,"%.*s", t[i+1].end-t[i+1].start,JSON_STRING + t[i+1].start);				
				//displayMessageWt("ResponseDescription",Cel_responseMessage);
				i++;
			}

			else if (jsoneq(JSON_STRING, &t[i], "ResponseCode") == 0) {
				memset(Cel_responseCode, 0, sizeof(Cel_responseCode));
				sprintf(Cel_responseCode,"%.*s", t[i+1].end-t[i+1].start,JSON_STRING + t[i+1].start);				
				//displayMessageWt("ResponseDescription",Cel_responseMessage);
				i++;
			}

	//"responseCode":"01","message		
			else if (jsoneq(JSON_STRING, &t[i], "token") == 0) {
				memset(Cel_Token, 0, sizeof(Cel_Token));
				sprintf(Cel_Token,"%.*s", t[i+1].end-t[i+1].start,JSON_STRING + t[i+1].start);	
				
				memset(eedc_token, 0, sizeof(eedc_token));
				sprintf(eedc_token,"%s",Cel_Token);				
				//displayMessageWt("ResponseDescription",Cel_responseMessage);
				i++;
			}
			
			else if (jsoneq(JSON_STRING, &t[i], "firstName") == 0) {
				memset(Cel_Fname, 0, sizeof(Cel_Fname));
				sprintf(Cel_Fname,"%.*s", t[i+1].end-t[i+1].start,JSON_STRING + t[i+1].start);				
				//displayMessageWt("ResponseDescription",Cel_responseMessage);
				i++;
			}

			else if (jsoneq(JSON_STRING, &t[i], "lastName") == 0) {
				memset(Cel_Lname, 0, sizeof(Cel_Lname));
				sprintf(Cel_Lname,"%.*s", t[i+1].end-t[i+1].start,JSON_STRING + t[i+1].start);				
				//displayMessageWt("ResponseDescription",Cel_responseMessage);
				i++;
			}
//PostCelTinValidateTransferReceived:{"responseCode":"00","message":"Validation Successful","transactionId":"61e0022118481f15d343b0b2","response":{"error":false,"name":"SANUSI SEGUN","message":"Approved or completed successfully","vendorBankCode":"000013","account":"0124936659","BankVerificationNumber":"22150192637","mod":424,"responseCode":"00"}}, Length: 312

			else if (jsoneq(JSON_STRING, &t[i], "agentId") == 0) {
				memset(Cel_AgentID, 0, sizeof(Cel_AgentID));
				sprintf(Cel_AgentID,"%.*s", t[i+1].end-t[i+1].start,JSON_STRING + t[i+1].start);				
				//displayMessageWt("ResponseDescription",Cel_responseMessage);
				i++;
			}
	//"userData"		
			
			else if (jsoneq(JSON_STRING, &t[i], "Amount") == 0) {
				
				memset(Cel_BenAmunt, 0, sizeof(Cel_BenAmunt));
				sprintf(Cel_BenAmunt,"%.*s", t[i+1].end-t[i+1].start,JSON_STRING + t[i+1].start);				
				//displayMessageWt("Amount",Cel_BenAmunt);
				i++;
			}else if (jsoneq(JSON_STRING, &t[i], "TransactionId") == 0) {
				
				memset(Cel_TransactionId, 0, sizeof(Cel_TransactionId));
				sprintf(Cel_TransactionId,"%.*s", t[i+1].end-t[i+1].start,JSON_STRING + t[i+1].start);				
				///displayMessageWt("TransactionId",Cel_TransactionId);
				i++;
			}

			else if (jsoneq(JSON_STRING, &t[i], "transactionId") == 0) {
				
				memset(Cel_TransactionId, 0, sizeof(Cel_TransactionId));
				sprintf(Cel_TransactionId,"%.*s", t[i+1].end-t[i+1].start,JSON_STRING + t[i+1].start);				
				///displayMessageWt("TransactionId",Cel_TransactionId);
				i++;
			}
			//transactionId
			else if (jsoneq(JSON_STRING, &t[i], "name") == 0) {
				
				memset(Cel_AccountName, 0, sizeof(Cel_AccountName));
				sprintf(Cel_AccountName,"%.*s", t[i+1].end-t[i+1].start,JSON_STRING + t[i+1].start);				
				///displayMessageWt("TransactionId",Cel_TransactionId);
				i++;
			}
			else if (jsoneq(JSON_STRING, &t[i], "PlatformId") == 0) {
				
				memset(Cel_PlatformID, 0, sizeof(Cel_PlatformID));
				sprintf(Cel_PlatformID,"%.*s", t[i+1].end-t[i+1].start,JSON_STRING + t[i+1].start);				
				///displayMessageWt("TransactionId",Cel_TransactionId);
				i++;
			}
		
//EEDC VALRIABLES

			else if (jsoneq(JSON_STRING, &t[i], "transaction_reference") == 0) {
				
				memset(eedc_transaction_reference, 0, sizeof(eedc_transaction_reference));
				sprintf(eedc_transaction_reference,"%.*s", t[i+1].end-t[i+1].start,JSON_STRING + t[i+1].start);				
				///displayMessageWt("TransactionId",Cel_TransactionId);
				i++;
			}

			else if (jsoneq(JSON_STRING, &t[i], "units") == 0) {
				
				memset(eedc_units, 0, sizeof(eedc_units));
				sprintf(eedc_units,"%.*s", t[i+1].end-t[i+1].start,JSON_STRING + t[i+1].start);				
				///displayMessageWt("TransactionId",Cel_TransactionId);
				i++;
			}

			else if (jsoneq(JSON_STRING, &t[i], "appliedToArrears") == 0) {
				
				memset(eedc_appliedToArrears, 0, sizeof(eedc_appliedToArrears));
				sprintf(eedc_appliedToArrears,"%.*s", t[i+1].end-t[i+1].start,JSON_STRING + t[i+1].start);				
				///displayMessageWt("TransactionId",Cel_TransactionId);
				i++;
			}

			

			else if (jsoneq(JSON_STRING, &t[i], "vat") == 0) {
				
				memset(eedc_vat, 0, sizeof(eedc_vat));
				sprintf(eedc_vat,"%.*s", t[i+1].end-t[i+1].start,JSON_STRING + t[i+1].start);				
				///displayMessageWt("TransactionId",Cel_TransactionId);
				i++;
			}




			else if (jsoneq(JSON_STRING, &t[i], "customerName") == 0) {
				
				memset(eedc_customerName, 0, sizeof(eedc_customerName));
				sprintf(eedc_customerName,"%.*s", t[i+1].end-t[i+1].start,JSON_STRING + t[i+1].start);				
				///displayMessageWt("TransactionId",Cel_TransactionId);
				i++;
			}

			else if (jsoneq(JSON_STRING, &t[i], "convenience") == 0) {
				
				memset(eedc_convenience, 0, sizeof(eedc_convenience));
				sprintf(eedc_convenience,"%.*s", t[i+1].end-t[i+1].start,JSON_STRING + t[i+1].start);				
				///displayMessageWt("TransactionId",Cel_TransactionId);
				i++;
			}

			else if (jsoneq(JSON_STRING, &t[i], "total") == 0) {
				
				memset(eedc_total, 0, sizeof(eedc_total));
				sprintf(eedc_total,"%.*s", t[i+1].end-t[i+1].start,JSON_STRING + t[i+1].start);				
				///displayMessageWt("TransactionId",Cel_TransactionId);
				i++;
			}
			

		//JAMB customerName otherCustomerInfo jamb_customerName


			else if (jsoneq(JSON_STRING, &t[i], "otherCustomerInfo") == 0) {
				
				memset(jamb_customerName, 0, sizeof(jamb_customerName));
				sprintf(jamb_customerName,"%.*s", t[i+1].end-t[i+1].start,JSON_STRING + t[i+1].start);				
				///displayMessageWt("TransactionId",Cel_TransactionId);
				i++;
			}


			//BankVerificationNumber

	}

	
	return 0;//EXIT_SUCCESS;
}


