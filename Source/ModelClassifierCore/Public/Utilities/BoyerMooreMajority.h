#pragma once

class FBoyerMooreMajority
{
public:
	template<typename T>
	static T FindMajorityOrFirst(const TArray<T>& Vote)
	{
		if (Vote.Num() == 0)
		{
			return T(); // empty
		}
		
		T Candidate;
		int32 VoteCount = 0;
		
		for (const T& Item : Vote)
		{
			if (VoteCount == 0)
			{
				Candidate = Item;
				VoteCount = 1;
			}
			else if (Item == Candidate)
			{
				++VoteCount;
			}
			else
			{
				--VoteCount;
			}
		}
		
		int32 Occurrences = 0;
		for (const T& Item : Vote)
		{
			if (Item == Candidate)
			{
				++Occurrences;
			}
		}
		
		const int32 N = Vote.Num();
		if (Occurrences > N / 2)
		{
			return Candidate;
		}

		return Vote[0];
	}
};
