#pragma once


std::vector<double> valueData = {
	0.0,
	7.5,
	15.0,
	22.5,
	30.0,
	37.75,
	45.25,
	52.75,
	60.25,
	67.75,
	75.25,
	82.75,
	90.25,
	98.0,
	105.5,
	113.0,
	120.5,
	128.0,
	135.5,
	143.0,
	150.5,
	158.0,
	165.75,
	173.25,
	180.75,
	188.25,
	195.75,
	203.25,
	210.75,
	218.25,
	226.0,
	233.5,
	241.0,
	248.5,
};

std::vector<double> aData = {
		0.4125709533691406 ,
		0.43141937255859375,
		0.4502677917480469,
		0.4607391357421875,
		0.47330474853515625,
		0.4795875549316406,
		0.4921531677246094,
		0.49843597412109375,
		0.5005302429199219,
		0.5005302429199219,
		0.50262451171875,
		0.49843597412109375,
		0.49005889892578125,
		0.4795875549316406,
		0.4774932861328125,
		0.46492767333984375,
		0.4544563293457031,
		0.4460792541503906,
		0.4293251037597656,
		0.4209480285644531,
		0.4083824157714844,
		0.3874397277832031,
		0.351837158203125,
		0.3288002014160156,
		0.2952919006347656,
		0.2534065246582031,
		0.19476699829101562,
		0.117279052734375,
		0.0,
		0.0,
		0.0,
		0.0,
		0.0,
		0.0 };

std::vector<double> bData = {
	0.2392120361328125,
	0.23725128173828125,
	0.23529052734375,
	0.23431015014648438,
	0.23529052734375,
	0.23627090454101562,
	0.23725128173828125,
	0.23823165893554688,
	0.24019241333007812,
	0.24019241333007812,
	0.23823165893554688,
	0.23725128173828125,
	0.23431015014648438,
	0.23234939575195312,
	0.2313690185546875,
	0.22842788696289062,
	0.22940826416015625,
	0.22842788696289062,
	0.22548675537109375,
	0.22254562377929688,
	0.21274185180664062,
	0.1999969482421875,
	0.17940902709960938,
	0.15587997436523438,
	0.133331298828125,
	0.11078262329101562,
	0.08039093017578125,
	0.04117584228515625,
	0.0,
	0.0,
	0.0,
	0.0,
	0.0,
	0.0 };

std::vector<double> kData = {
	0.0,
	0.0,
	-0.0017337799072265625,
	0.07975387573242188,
	0.14910507202148438,
	0.1803131103515625,
	0.2080535888671875,
	0.2305927276611328,
	0.2531318664550781,
	0.27740478515625,
	0.29821014404296875,
	0.3224830627441406,
	0.34848976135253906,
	0.3744964599609375,
	0.3987693786621094,
	0.42824363708496094,
	0.4542503356933594,
	0.4767894744873047,
	0.5114650726318359,
	0.5357379913330078,
	0.5634784698486328,
	0.5894851684570312,
	0.6276283264160156,
	0.6588363647460938,
	0.7004470825195312,
	0.7420578002929688,
	0.7888698577880859,
	0.8460845947265625,
	0.8876953125,
	0.0,
	0.0,
	0.0,
	0.0,
	0.0 };

std::vector<double> getAData() { return aData; }
std::vector<double> getBData() { return bData; }
std::vector<double> getKData() { return kData; }
std::vector<double> getValueData() { return valueData; }