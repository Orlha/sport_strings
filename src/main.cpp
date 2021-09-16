#include <unicode/unistr.h>
#include <unicode/normalizer2.h>
#include <unicode/normlzr.h>
#include <unicode/utypes.h>
#include <string>
#define FMT_HEADER_ONLY
#include <fmt/format.h>
#include "edit_distance.hpp"
#include "test_data.hpp"
#include <stdexcept>

namespace config {
constexpr auto validity_threshold = 0.7;
}


template<typename T>
class StrategyProcessor {
	public:
	DataBlock process(const DataBlock& data_block) {
		DataBlock result {
			T{}.process(data_block.a),
			T{}.process(data_block.b)
		};
		return result;
	}
};

struct Transliterate {
	private:
	friend StrategyProcessor<Transliterate>;
	constexpr static auto letters_size = 33;
	constexpr static const char* russian[letters_size] = {
		"а", "б", "в", "г", "д", "е", "ё", "ж", "з", "и", "й", "к", "л", "м", "н", "о", "п", "р", "с", "т", "у", "ф", "х", "ц", "ч", "ш", "щ", "ъ", "ы", "ь", "э", "ю", "я",
	};
	constexpr static const char* english[letters_size] = {
		"a", "b", "v", "g", "d", "e", "yo", "g", "z", "i", "y", "k", "l", "m", "n", "o", "p", "r", "s", "t", "u", "f", "kh", "ts", "ch", "sh", "sch", "", "y", "", "a", "yu", "ya",
		//"a", "b", "v", "g", "d", "e", "yo", "zh", "z", "i", "y", "k", "l", "m", "n", "o", "p", "r", "s", "t", "u", "f", "kh", "ts", "ch", "sh", "sch", "", "y", "", "e", "yu", "ya",
	};
	std::string process(const std::string& data) {
		icu::UnicodeString us(data.c_str(), "UTF-8");
		for(auto i = 0u; i < letters_size; ++i) {
			us.findAndReplace(russian[i], english[i]);
		}
		std::string result;
		us.toUTF8String(result);
		return result;
	}
};

struct Reduce {
	private:
	friend StrategyProcessor<Reduce>;
	std::string process(const std::string& data) {
		icu::UnicodeString us(data.c_str(), "UTF-8");
		us.findAndReplace(".", "");
		us.findAndReplace(",", "");
		us.findAndReplace("-", " ");
		std::string result;
		us.toUTF8String(result);
		return result;
	}
};

struct Swap {
	private:
	friend StrategyProcessor<Swap>;
	std::string process(const std::string& data) {
		auto middle = data.find(" ");
		auto start = data.substr(0, middle);
		auto end = data.substr(middle + 1);
		return end + " " + start;
	}
};

struct Lower {
	private:
	friend StrategyProcessor<Lower>;
	std::string process(const std::string& data) {
		icu::UnicodeString us(data.c_str(), "UTF-8");
		us.toLower();
		std::string result;
		us.toUTF8String(result);
		return result;
	}
};

struct Phonemize {
	private:
	friend StrategyProcessor<Phonemize>;
	std::string process(const std::string& data) {
		return data;
	}
};

struct Normalize {
	private:
	friend StrategyProcessor<Normalize>;
	std::string process(const std::string& data) {
#if 0
		icu::UnicodeString us(data.c_str(), "UTF-8");
		icu::UnicodeString ds;
		//icu::Normalizer::normalize(us, UNORM_DEFAULT, 0, ds, err);
		UErrorCode err = U_ZERO_ERROR;
		auto f = icu::Normalizer2::getNFCInstance(err);
		//std::printf("error code %04X\n", err);
		if(err <= U_ZERO_ERROR) {
			//fmt::print("okay\n");
			f->normalize(us, ds, err);
			std::string result;
			ds.toUTF8String(result);
			return result;
		} else {
			throw 1;
		}
#else
		return data;
#endif
	}
};

double calculate_similarity(const std::string& a, const std::string& b) {
	auto distance = utility::levenshtein_distance(a, b, 1, 0, 1);
	auto length = a.size() > b.size() ? a.size() : b.size();
	auto similarity = 1.0 - static_cast<double>(distance) / static_cast<double>(length);
	return similarity;
}

bool check_validity(double value) {
	return value >= config::validity_threshold;
}

double fuzzy_compare(const std::string& a, const std::string& b) {
	const auto split_token = " vs ";
	auto split = [&] (const auto& input) {
		auto middle = input.find(split_token);
		if(middle == std::string::npos) {
			throw 1;
		}
		DataBlock data {
			input.substr(0, middle),
			input.substr(middle + 4)
		};
		return data;
	};
	const auto start_data1 = split(a);
	const auto start_data2 = split(b);
	auto data1 = StrategyProcessor<Lower>{}.process(start_data1);
	auto data2 = StrategyProcessor<Lower>{}.process(start_data2);
	data1 = StrategyProcessor<Reduce>{}.process(data1);
	data2 = StrategyProcessor<Reduce>{}.process(data2);
	data1 = StrategyProcessor<Normalize>{}.process(data1);
	data2 = StrategyProcessor<Normalize>{}.process(data2);
	data1 = StrategyProcessor<Phonemize>{}.process(data1);
	data2 = StrategyProcessor<Phonemize>{}.process(data2);
	data1 = StrategyProcessor<Transliterate>{}.process(data1);
	data2 = StrategyProcessor<Transliterate>{}.process(data2);
	auto sim1 = calculate_similarity(data1.a, data2.a);
	auto sim2 = calculate_similarity(data1.b, data2.b);
	if(not check_validity(sim1) || not check_validity(sim2)) {
		std::swap(data1.a, data1.b);
		sim1 = calculate_similarity(data1.a, data2.a);
		sim2 = calculate_similarity(data1.b, data2.b);
	}
	if(not check_validity(sim1) || not check_validity(sim2)) {
		std::swap(data1.a, data1.b);
		data1 = StrategyProcessor<Swap>{}.process(data1);
		sim1 = calculate_similarity(data1.a, data2.a);
		sim2 = calculate_similarity(data1.b, data2.b);
	}
	if(not check_validity(sim1) || not check_validity(sim2)) {
		data1 = StrategyProcessor<Swap>{}.process(data1);
		sim1 = calculate_similarity(data1.a, data2.a);
		sim2 = calculate_similarity(data1.b, data2.b);
	}
	fmt::print("Resulting sets\n");
	fmt::print("Set 1: {} vs {}\n", data1.a, data1.b);
	fmt::print("Set 2: {} vs {}\n", data2.a, data2.b);
	return sim1 < sim2 ? sim1 : sim2;
}

void check_test_inputs() {
	for(const auto& [a, b]: test::correct_inputs) {
		auto result = fuzzy_compare(a, b);
		if(not check_validity(result)) {
			fmt::print("[FAILURE] Compare result of \"{}\" and \"{}\" is [{}]\n", a, b, result);
		} else {
			//fmt::print("[SUCCESS] Compare result of \"{}\" and \"{}\" is [{}]\n", a, b, result);
		}
	}
}

int main() {
	check_test_inputs();
}
