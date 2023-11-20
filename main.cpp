#pragma warning(disable : 4996).
#define CURL_STATICLIB
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <ctime>
#include "Dependencies/include/json.hpp"
#include "Dependencies/include/curl.h"
#include "Dependencies/include/type_conversion.hh"


// FIXING CURL POTENTIALLY https://stackoverflow.com/questions/47878888/getting-curl-to-work-with-visual-studios-2017
// HTML C++ WEBSITE https://www.youtube.com/watch?v=lh3MGxP3Rl8&t=658s
//https://www.youtube.com/watch?v=ThNTJFBYL0Q
//WRITNG TO JSON https://json.nlohmann.me/features/arbitrary_types/


//JSON NAMESPACE


//PROTYPING 
static size_t my_write(void* buffer, size_t size, size_t nmemb, void* param)
{
	std::string& text = *static_cast<std::string*>(param);
	size_t totalsize = size * nmemb;
	text.append(static_cast<char*>(buffer), totalsize);
	return totalsize;
}
size_t _null_write(char* ptr, size_t size, size_t nmemb, void* userdata) {
	return size * nmemb;
}
std::string validatestock(std::string ticker, float investmentAmount);
int twilloMsgSend(std::string msg, std::string phone);
void runAlgo(std::string name);

int main(){
	std::ifstream ifs("users.json");
	nlohmann::json j = nlohmann::json::parse(ifs);

	time_t now = time(0);
	tm* ltm = localtime(&now);
	std::stringstream dateStream;
	dateStream << "/" << ltm->tm_mday << "/" << 1900 + ltm->tm_year;

	for (auto& e : j["user"].items()) {
		for (auto el : j["user"][e.key()]["investDate"].items()) {
			if (dateStream.str() == el.value()) {
				runAlgo(e.key());
			}

		}
	}
	return 0;
}


std::string validatestock(std::string ticker, float investmentAmount) {
	/////////-- ALGORITHIM --/////////
	// CUSTOM URL TO YAHOOFINANCE
	std::string _interval("1wk");
	std::string url = "http://query1.finance.yahoo.com/v7/finance/download/" + ticker + "?period1=0&period2=9999999999" + "&interval=" + _interval + "&events=history";

	//CURL 
	std::string result;
	CURL* curl;
	CURLcode res;
	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_write);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
		//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, true);
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
		if (CURLE_OK != res) {
			std::cerr << "CURL error: " << res << '\n';
		}
	}
	curl_global_cleanup();


	// CURL CSV TO VECTOR
	std::string datestr, openstr, highstr, lowstr, closestr, adjclosestr, volumestr, line;
	std::vector<int> date;
	std::vector<float> open;
	std::vector<float> high;
	std::vector<float> low;
	std::vector<float> close;
	std::vector<float> adjclose;
	std::vector<float> volume;
	std::stringstream sso(result);
	sso.ignore(256, '\n');
	while (!sso.eof()) {
		while (std::getline(sso, line) && !line.empty() && line.find("null") == std::string::npos) {
			std::stringstream ss(line);
			std::getline(ss, datestr, ',');
			date.push_back(stoi(datestr));
			std::getline(ss, openstr, ',');
			open.push_back(stof(openstr));
			std::getline(ss, highstr, ',');
			high.push_back(stof(highstr));
		}
	};

	// Find PERCENTAGE CHANGE VECTOR
	std::vector<float> perChaVect;

	// USING HIGH value To account for all 
	std::vector<float> unsortedperChaVect;
	// Find PERCENTAGE CHANGE INTERVAL
	std::vector<float> perChaInterval;

	//// ------- PLEASE OPTIMISE -------- ////
	for (int i = 1; i < open.size(); i++) {
		unsortedperChaVect.push_back(((high.at(i) - high.at(i - 1)) / high.at(i - 1)) * 100);
		perChaVect.push_back(((high.at(i) - high.at(i - 1)) / high.at(i - 1)) * 100);
	}
	// Sorting PERCENTAGE CHANGE VECTOR LOW TO HIGH
	sort(perChaVect.begin(), perChaVect.end());

	int perChaInt = 100 + 2;
	float greaterThanZero = 0;

	for (int i = 0; i < perChaVect.size(); i++) {
		if (perChaVect.at(i) == 0 || ((i > 1) && (perChaVect.at(i - 1) < 0) && (perChaVect.at(i) > 0))) {
			greaterThanZero = perChaVect.at(i + 1);
		}
	}


	//FIND INCREMENT AMOUNT
	float intervalPercentageindex1 = (greaterThanZero + perChaVect.back()) / perChaInt;
	float intervalPercentage = 0;

	for (int i = 0; i < perChaInt; i++) {
		intervalPercentage += intervalPercentageindex1;
		perChaInterval.push_back(intervalPercentage);
	}


	//Find FREQUENCY OF PERCENTAGE CHANGE INTERVALS

	std::vector<float> frePerChaIntervals;
	std::vector<float> AvgPerChaInterval;
	std::vector<float> sumPerChaInterval;
	std::vector<float> percentAnnualROI;
	std::vector<float> successIndex;


	float perChaVectSize = 0;
	int sizeofInterval = 0;
	float freAvgPerCha = 0;
	float max = 0;
	float sum = 0;
	float sumChaVect = 0;
	float yearCompanyLength = date.back() - date.front();

	// ----- NOTE OF REALISATION [CHECK FOR OPTIMISITION] FOR(VECTOR INTERATOR) vs FOR (int i) :: THOUGHT .BEGIN() RANDOM;

	for (int i = 0; i < perChaInterval.size(); i++) {
		for (int j = 1; j < perChaVect.size(); j++) {
			if (perChaVect.at(j) < perChaInterval.at(i)) {
				sum += perChaVect.at(j);
				++sizeofInterval;
			}
			++perChaVectSize;
			sumChaVect += perChaVect.at(j);
		}

		sumPerChaInterval.push_back(sum);

		AvgPerChaInterval.push_back(-(sumPerChaInterval.at(i)) / sizeofInterval);

		freAvgPerCha = (sizeofInterval / perChaVectSize / 52);

		frePerChaIntervals.push_back(freAvgPerCha);

		percentAnnualROI.push_back(AvgPerChaInterval.at(i) * freAvgPerCha);

		// find SUCESSINDEX
		successIndex.push_back((percentAnnualROI.at(i) * (frePerChaIntervals.at(i))) / 2);

		// find MAX ELEMENT IN SUCESSINDEX
		max = *std::max_element(successIndex.begin(), successIndex.end());


		sum = 0;
		sumChaVect = 0;
		sizeofInterval = 0;
		freAvgPerCha = 0;
		perChaVectSize = 0;
	}
	// find Index of Max Value in SucessIdex
	auto it = find(successIndex.begin(), successIndex.end(), max);
	int index = 0;
	if (it != successIndex.end())
	{
		index = it - successIndex.begin();
	}

	// converting INDEX INTO PERCENTAGE VALUE
	// Trading ANY STOCK THAT HAS PERCENTAGE CHANGE BELOW THRESHOLD
	float threshold = AvgPerChaInterval.at(index);
	std::string resultString;

	if (threshold > unsortedperChaVect.back()) {

		resultString = "INVEST";
	}
	else {

		resultString = "HOLD";
	}
	return resultString;
};


int twilloMsgSend(std::string msg, std::string phone) {

	std::string account_sid = "AC702b02183b2fd41beb241015f5dd0fc7";
	std::string auth_token = "00d991d1c703e9213f7b0ad25b0b39e9";
	std::string message = msg;
	std::string from_number = "+13237161145";
	std::string to_number = phone;
	std::string picture_url;
	bool verbose = false;

	// Instantiate a twilio object and call send_message
	std::string response;
	std::stringstream response_stream;
	std::u16string converted_message_body;

	converted_message_body = utf8_to_ucs2(msg);
	if (converted_message_body.size() > 1600) {
		response_stream << "Message body must have 1600 or fewer" << " characters. Cannot send message with " << converted_message_body.size() << " characters.";
		response = response_stream.str();
		return false;
	}
	//CURL
	CURL* curl;
	CURLcode res;
	std::stringstream parameters;
	std::string parameter_string;
	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	// Percent encode special characters
	char* message_body_escaped = curl_easy_escape(curl, msg.c_str(), 0);
	std::stringstream url;
	std::string url_string;
	url << "https://api.twilio.com/2010-04-01/Accounts/" << account_sid << "/Messages";
	url_string = url.str();
	parameters << "To=" << to_number << "&From=" << from_number << "&Body=" << message_body_escaped;
	if (!picture_url.empty()) {
		parameters << "&MediaUrl=" << picture_url;
	}
	parameter_string = parameters.str();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_POST, 1);
		curl_easy_setopt(curl, CURLOPT_URL, url_string.c_str());
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, parameter_string.c_str());
		curl_easy_setopt(curl, CURLOPT_USERNAME, account_sid.c_str());
		curl_easy_setopt(curl, CURLOPT_PASSWORD, auth_token.c_str());
		if (!verbose) {
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _null_write);
		}
		else {
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_write);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_stream);
		}

		curl_free(message_body_escaped);
		res = curl_easy_perform(curl);
		long http_code = 0;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
		if (CURLE_OK != res) {
			std::cerr << "CURL error: " << res << '\n';
		}
	}
	curl_global_cleanup();
}


void runAlgo(std::string name) {

	// INITIALZATION
	std::vector<std::string> watchlist;
	std::vector<float> watchlistWeight;
	float dollar = 0;
	float investmentAmount;
	std::stringstream ss;
	std::string txtMessage;

	std::ifstream ifs("users.json");
	nlohmann::json j = nlohmann::json::parse(ifs);

	ifs.close();


	std::string phone = j["user"][name]["phone"];
	dollar = j["user"][name]["investmentAmt"];

	std::string str;
	int sumWeight = 0;

	for (auto& e : j["user"][name]["watchlist"].items()) {
		watchlist.push_back(e.key());
		for (auto& el : j["user"][name]["watchlist"][e.key()].items()) {
			if (el.key() == "investmentWeight") {
				watchlistWeight.push_back(el.value());
				sumWeight += el.value();
			}
		}
	}
	// serialzation - Boost

	std::string algoDecision;
	float accBalance = 0;
	float infocount = 0;


	ss << std::fixed << std::setprecision(2) << "\nWeekly Investment Amount : " << dollar << std::endl;


	for (int i = 0; i < watchlist.size(); i++) {
		algoDecision = validatestock(watchlist.at(i), dollar);

		if (algoDecision == "HOLD") {
			investmentAmount = j["user"][name]["watchlist"][watchlist.at(i)]["unInvestedAmount"] + ((watchlistWeight.at(i) / sumWeight) * dollar);
			j["user"][name]["watchlist"][watchlist.at(i)]["unInvestedAmount"] = investmentAmount;
			ss << std::fixed << std::setprecision(2) << "    HOLD   |         | " << watchlist.at(i) << std::endl;
			accBalance += investmentAmount;
		}
		if (algoDecision == "INVEST") {
			investmentAmount = j["user"][name]["watchlist"][watchlist.at(i)]["unInvestedAmount"] + ((watchlistWeight.at(i) / sumWeight) * dollar);
			j["user"][name]["watchlist"][watchlist.at(i)]["unInvestedAmount"] = 0.00;
			ss << std::fixed << std::setprecision(2) << "    INVEST | $" << investmentAmount << " | " << watchlist.at(i) << std::endl;
			//ss << std::fixed << std::setprecision(2) << "    Invest $" << investmentAmount << " in " << watchlist.at(i) << std::endl;
			infocount += investmentAmount;
		}
	}
	std::ofstream file("users.json");
	file << j;

	ss << std::fixed << std::setprecision(2) << "\nRemaining Balance in Account : " << accBalance << std::endl;

	//BUILDING TEXT
	txtMessage = "STOCK ALERT - " + name  + "\n" + ss.str();

	std::stringstream infoStream;
	infoStream << std::fixed << std::setprecision(2) << infocount;

	std::string infotext = name + " : Invested in $" + infoStream.str();
	//+"/" + std::to_string(watchlist.size()) + "\n";
	//SENDING TEXT MESSAGE 
	std::cout << txtMessage << std::endl;
	std::cout << infotext << std::endl;
	//twilloMsgSend(txtMessage, phone);
	//twilloMsgSend(infotext, "8182874882");
};
