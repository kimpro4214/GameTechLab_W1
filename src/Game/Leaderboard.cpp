#include "pch.h"
#include "Game/Leaderboard.h"

#include <cctype>
#include <filesystem>
#include <fstream>
#include <iterator>

namespace
{
	constexpr std::size_t MaxLeaderboardEntries = 10;

	void SkipWhitespace(const std::string& Text, std::size_t& Position)
	{
		while (Position < Text.size() &&
			std::isspace(static_cast<unsigned char>(Text[Position])))
		{
			++Position;
		}
	}

	bool Consume(const std::string& Text, std::size_t& Position, char Expected)
	{
		SkipWhitespace(Text, Position);
		if (Position >= Text.size() || Text[Position] != Expected)
		{
			return false;
		}
		++Position;
		return true;
	}

	bool ParseString(const std::string& Text, std::size_t& Position, std::string& Result)
	{
		if (!Consume(Text, Position, '"'))
		{
			return false;
		}

		Result.clear();
		while (Position < Text.size())
		{
			const char Character = Text[Position++];
			if (Character == '"')
			{
				return true;
			}
			if (Character != '\\' || Position >= Text.size())
			{
				Result += Character;
				continue;
			}

			const char EscapedCharacter = Text[Position++];
			switch (EscapedCharacter)
			{
			case '"': Result += '"'; break;
			case '\\': Result += '\\'; break;
			case '/': Result += '/'; break;
			case 'b': Result += '\b'; break;
			case 'f': Result += '\f'; break;
			case 'n': Result += '\n'; break;
			case 'r': Result += '\r'; break;
			case 't': Result += '\t'; break;
			default: return false;
			}
		}
		return false;
	}

	bool ParseInteger(const std::string& Text, std::size_t& Position, int& Result)
	{
		SkipWhitespace(Text, Position);
		const std::size_t Start = Position;
		if (Position < Text.size() && Text[Position] == '-')
		{
			++Position;
		}
		while (Position < Text.size() && std::isdigit(static_cast<unsigned char>(Text[Position])))
		{
			++Position;
		}
		if (Start == Position || (Text[Start] == '-' && Start + 1 == Position))
		{
			return false;
		}

		try
		{
			Result = std::stoi(Text.substr(Start, Position - Start));
			return true;
		}
		catch (...)
		{
			return false;
		}
	}

	std::string EscapeJsonString(const std::string& Value)
	{
		std::string Escaped;
		for (const char Character : Value)
		{
			switch (Character)
			{
			case '"': Escaped += "\\\""; break;
			case '\\': Escaped += "\\\\"; break;
			case '\n': Escaped += "\\n"; break;
			case '\r': Escaped += "\\r"; break;
			case '\t': Escaped += "\\t"; break;
			default: Escaped += Character; break;
			}
		}
		return Escaped;
	}
}

Leaderboard::Leaderboard(std::string InFilePath)
	: FilePath(std::move(InFilePath))
{
}

void Leaderboard::Load()
{
	Entries.clear();
	std::ifstream File(FilePath);
	if (!File)
	{
		return;
	}

	const std::string Text(
		(std::istreambuf_iterator<char>(File)),
		std::istreambuf_iterator<char>());
	std::size_t Position = 0;
	if (!Consume(Text, Position, '['))
	{
		return;
	}

	SkipWhitespace(Text, Position);
	while (Position < Text.size() && Text[Position] != ']')
	{
		std::string Key;
		std::string Name;
		int Score = 0;
		bool bHasName = false;
		bool bHasScore = false;
		if (!Consume(Text, Position, '{'))
		{
			Entries.clear();
			return;
		}

		while (true)
		{
			if (!ParseString(Text, Position, Key) || !Consume(Text, Position, ':'))
			{
				Entries.clear();
				return;
			}
			if (Key == "name")
			{
				bHasName = ParseString(Text, Position, Name);
			}
			else if (Key == "score")
			{
				bHasScore = ParseInteger(Text, Position, Score);
			}
			else
			{
				Entries.clear();
				return;
			}
			if ((!bHasName && Key == "name") || (!bHasScore && Key == "score"))
			{
				Entries.clear();
				return;
			}

			SkipWhitespace(Text, Position);
			if (Position < Text.size() && Text[Position] == '}')
			{
				++Position;
				break;
			}
			if (!Consume(Text, Position, ','))
			{
				Entries.clear();
				return;
			}
		}

		if (bHasName && bHasScore)
		{
			Entries.push_back({ Name, Score });
		}
		SkipWhitespace(Text, Position);
		if (Position < Text.size() && Text[Position] != ']' && !Consume(Text, Position, ','))
		{
			Entries.clear();
			return;
		}
		SkipWhitespace(Text, Position);
	}

	if (!Consume(Text, Position, ']'))
	{
		Entries.clear();
		return;
	}
	SortAndTrim();
}

bool Leaderboard::Add(const std::string& Name, int Score)
{
	Entries.push_back({ Name, Score });
	SortAndTrim();
	return Save();
}

bool Leaderboard::Save() const
{
	const std::filesystem::path Path(FilePath);
	std::error_code Error;
	if (Path.has_parent_path())
	{
		std::filesystem::create_directories(Path.parent_path(), Error);
		if (Error)
		{
			return false;
		}
	}

	std::ofstream File(FilePath, std::ios::trunc);
	if (!File)
	{
		return false;
	}

	File << "[\n";
	for (std::size_t Index = 0; Index < Entries.size(); ++Index)
	{
		const LeaderboardEntry& Entry = Entries[Index];
		File << "  { \"name\": \"" << EscapeJsonString(Entry.Name)
			<< "\", \"score\": " << Entry.Score << " }";
		File << (Index + 1 < Entries.size() ? ",\n" : "\n");
	}
	File << "]\n";
	return File.good();
}

void Leaderboard::SortAndTrim()
{
	std::sort(Entries.begin(), Entries.end(),
		[](const LeaderboardEntry& Left, const LeaderboardEntry& Right)
		{
			return Left.Score > Right.Score;
		});
	if (Entries.size() > MaxLeaderboardEntries)
	{
		Entries.resize(MaxLeaderboardEntries);
	}
}
