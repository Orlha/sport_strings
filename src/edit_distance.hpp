#pragma once
#include <vector>

namespace utility {
template<typename T>
	typename T::size_type levenshtein_distance(const T& source,
			const T& target,
			typename T::size_type insert_cost = 1,
			typename T::size_type delete_cost = 1,
			typename T::size_type replace_cost = 1) {
#if 0
		if (source.size() > target.size()) {
			return levenshtein_distance(target, source, delete_cost, insert_cost, replace_cost);
		}
#else
		if (source.size() < target.size()) {
			return levenshtein_distance(target, source, insert_cost, delete_cost, replace_cost);
		}
#endif

		using size_type = typename T::size_type;
		const size_type min_size = source.size(), max_size = target.size();
		std::vector<size_type> lev_dist(min_size + 1);

		lev_dist[0] = 0;
		for (size_type i = 1; i <= min_size; ++i) {
			lev_dist[i] = lev_dist[i - 1] + delete_cost;
		}

		for (size_type j = 1; j <= max_size; ++j) {
			size_type previous_diagonal = lev_dist[0], previous_diagonal_save;
			lev_dist[0] += insert_cost;

			for (size_type i = 1; i <= min_size; ++i) {
				previous_diagonal_save = lev_dist[i];
				if (source[i - 1] == target[j - 1]) {
					lev_dist[i] = previous_diagonal;
				} else {
					lev_dist[i] = std::min(std::min(lev_dist[i - 1] + delete_cost, lev_dist[i] + insert_cost), previous_diagonal + replace_cost);
				}
				previous_diagonal = previous_diagonal_save;
			}
		}

		return lev_dist[min_size];
	}
}
