#pragma once

#include <string>
#include <vector>

struct LeaderboardEntry
{
	std::string Name;
	int Score = 0;
};

class Leaderboard
{
public:
	explicit Leaderboard(std::string InFilePath = "data/leaderboard.json");

	void Load();
	bool Add(const std::string& Name, int Score);
	const std::vector<LeaderboardEntry>& GetEntries() const { return Entries; }

private:
	bool Save() const;
	void SortAndTrim();

	std::string FilePath;
	std::vector<LeaderboardEntry> Entries;
};
