/* Profile - Player data that persists between sessions.  Can be stored on a local disk or on a memory card. */

#ifndef Profile_H
#define Profile_H

#include "GameConstantsAndTypes.h"
#include "Style.h"
#include "Grade.h"
#include <map>
#include <set>
#include "HighScore.h"
#include "TimeConstants.h"
#include "SongUtil.h"	// for SongID
#include "StepsUtil.h"	// for StepsID
#include "CourseUtil.h"	// for CourseID
#include "TrailUtil.h"	// for TrailID

struct XNode;

//
// Current file versions
//
const CString STATS_XML				= "Stats.xml";

const CString EDITABLE_INI			= "Editable.ini";
// Editable data is an INI because the default INI file association on Windows 
// systems will open the ini file in an editor.  The default association for 
// XML will open in IE.  Users have a much better chance of discovering how to 
// edit this data if they don't have to fight against the file associations.

const CString DONT_SHARE_SIG		= "DontShare.sig";
// The "don't share" file is something that the user should always keep private.
// They can safely share STATS_XML with STATS_XML's signature so that others
// can authenticate the STATS_XML data.  However, others can't copy that data
// to their own profile for use in the game unless they also have the "don't 
// share" file.  DontShare contains a piece of information that we can 
// construct using STATS_XML but the user can't construct using STATS_XML.
// The file contains a signature of the STATS_XML's signature.

const CString PUBLIC_KEY_FILE		= "public.key";
const CString SCREENSHOTS_SUBDIR	= "Screenshots/";
const CString EDITS_SUBDIR			= "Edits/";


class Song;
class Steps;
class Course;

class Profile
{
public:
	Profile()
	{
		InitAll();
	}

	//
	// smart accessors
	//
	CString GetDisplayName() const;
	CString GetDisplayTotalCaloriesBurned() const;
	CString GetDisplayTotalCaloriesBurnedToday() const;
	int GetTotalNumSongsPlayed() const;
	int GetTotalNumSongsPassed() const;
	int GetActualSongDancePoints( StepsType st, Difficulty dc ) const;
	int GetActualCourseDancePoints( StepsType st, CourseDifficulty cd ) const;
	int GetPossibleSongDancePoints( StepsType st, Difficulty dc ) const;
	int GetPossibleCourseDancePoints( StepsType st, CourseDifficulty cd ) const;
	float GetSongsPercentComplete( StepsType st, Difficulty dc ) const { return (float)(GetActualSongDancePoints(st,dc))/GetPossibleSongDancePoints(st,dc); }
	float GetCoursesPercentComplete( StepsType st, CourseDifficulty cd ) const { return (float)(GetActualCourseDancePoints(st,cd))/GetPossibleCourseDancePoints(st,cd); }
	static CString GetProfileDisplayNameFromDir( CString sDir );
	int GetSongNumTimesPlayed( const Song* pSong ) const;
	int GetSongNumTimesPlayed( const SongID& songID ) const;
	
	void AddStepTotals( int iNumTapsAndHolds, int iNumJumps, int iNumHolds, int iNumMines, int iNumHands );

	bool IsMachine() const;

	//
	// Editable data
	//
	CString m_sDisplayName;
	CString m_sLastUsedHighScoreName;	// this doesn't really belong in "editable", but we need it in the smaller editable file so that it can be ready quickly.
	int m_iWeightPounds;

	//
	// General data
	//
	CString m_sGuid;
	bool m_bUsingProfileDefaultModifiers;
	CString m_sDefaultModifiers;
	SortOrder m_SortOrder;
	Difficulty m_LastDifficulty;
	CourseDifficulty m_LastCourseDifficulty;
	Song* m_pLastSong;
	Course* m_pLastCourse;
	int m_iTotalPlays;
	int m_iTotalPlaySeconds;
	int m_iTotalGameplaySeconds;
	int m_iCurrentCombo;
	float m_fTotalCaloriesBurned;
	int m_iTotalDancePoints;
	int m_iNumExtraStagesPassed;
	int m_iNumExtraStagesFailed;
	int m_iNumToasties;
	int m_iTotalTapsAndHolds;
	int m_iTotalJumps;
	int m_iTotalHolds;
	int m_iTotalMines;
	int m_iTotalHands;
	set<int> m_UnlockedSongs;
	mutable CString m_sLastPlayedMachineGuid;	// mutable because we overwrite this on save, and I don't want to remove const from the whole save chain. -Chris
	int m_iNumSongsPlayedByPlayMode[NUM_PLAY_MODES];
	int m_iNumSongsPlayedByStyle[NUM_STYLES];
	int m_iNumSongsPlayedByDifficulty[NUM_DIFFICULTIES];
	int m_iNumSongsPlayedByMeter[MAX_METER+1];
	int m_iNumSongsPassedByPlayMode[NUM_PLAY_MODES];
	int m_iNumSongsPassedByGrade[NUM_GRADES];

	//
	// Song high scores
	//
	struct HighScoresForASteps
	{
		HighScoreList hs;
	};
	struct HighScoresForASong
	{
		std::map<StepsID,HighScoresForASteps>	m_StepsHighScores;
	};
	std::map<SongID,HighScoresForASong>	m_SongHighScores;

	void AddStepsHighScore( const Song* pSong, const Steps* pSteps, HighScore hs, int &iIndexOut );
	const HighScoreList& GetStepsHighScoreList( const Song* pSong, const Steps* pSteps ) const;
	HighScoreList& GetStepsHighScoreList( const Song* pSong, const Steps* pSteps );
	int GetStepsNumTimesPlayed( const Song* pSong, const Steps* pSteps ) const;
	void IncrementStepsPlayCount( const Song* pSong, const Steps* pSteps );
	void GetGrades( const Song* pSong, StepsType st, int iCounts[NUM_GRADES] ) const;


	//
	// Course high scores
	//
	// struct was a typedef'd array of HighScores, but VC6 freaks out 
	// in processing the templates for map::operator[].
	struct HighScoresForATrail
	{
		HighScoreList hs;
	};
	struct HighScoresForACourse	
	{
		std::map<TrailID,HighScoresForATrail>	m_TrailHighScores;
	};
	std::map<CourseID,HighScoresForACourse>	m_CourseHighScores;

	void AddCourseHighScore( const Course* pCourse, const Trail* pTrail, HighScore hs, int &iIndexOut );
	HighScoreList& GetCourseHighScoreList( const Course* pCourse, const Trail* pTrail );
	const HighScoreList& GetCourseHighScoreList( const Course* pCourse, const Trail* pTrail ) const;
	int GetCourseNumTimesPlayed( const Course* pCourse ) const;
	int GetCourseNumTimesPlayed( const CourseID& courseID ) const;
	void IncrementCoursePlayCount( const Course* pCourse, const Trail* pTrail );


	//
	// Category high scores
	//
	HighScoreList m_CategoryHighScores[NUM_STEPS_TYPES][NUM_RANKING_CATEGORIES];

	void AddCategoryHighScore( StepsType st, RankingCategory rc, HighScore hs, int &iIndexOut );
	HighScoreList& GetCategoryHighScoreList( StepsType st, RankingCategory rc );
	const HighScoreList& GetCategoryHighScoreList( StepsType st, RankingCategory rc ) const;
	int GetCategoryNumTimesPlayed( StepsType st ) const;
	void IncrementCategoryPlayCount( StepsType st, RankingCategory rc );


	//
	// Screenshot Data
	//
	vector<Screenshot> m_vScreenshots;
	void AddScreenshot( const Screenshot &screenshot );
	int GetNextScreenshotIndex() { return m_vScreenshots.size(); }


	//
	// Calorie Data
	//
	// Why track calories in a map, and not in a static sized array like 
	// Bookkeeping?  The machine's clock is not guaranteed to be set correctly.
	// If calorie array is in a static sized array, playing on a machine with 
	// a mis-set clock could wipe out all your past data.  With this scheme, 
	// the worst that could happen is that playing on a mis-set machine will 
	// insert some garbage entries into the map.
	struct Day
	{
		int iDayInYear;	// 0-365
		int iYear;		// e.g. 2004
		bool operator==( const Day& other ) const { return iDayInYear==other.iDayInYear && iYear==other.iYear; }
		bool operator<( const Day& other ) const 
		{
			if(iYear<other.iYear) 
				return true; 
			if( iYear>other.iYear) 
				return false;
			else
				return iDayInYear<other.iDayInYear;
		}
	};
	map<Day,float> m_mapDayToCaloriesBurned;
	float GetCaloriesBurnedForDay( Day day ) const;
	
	//
	// Awards
	//
	struct AwardRecord
	{
		time_t first;
		time_t last;
		int iCount;	// num times achieved

		AwardRecord() { Unset(); }

		bool IsSet() const	{ return iCount>0; }
		void Set(time_t t)	{ if (iCount==0) first = t; last = t; iCount++; }
		void Unset()		{ iCount = 0; first = -1; last = -1; }
		
		XNode* CreateNode() const;
		void LoadFromNode( const XNode* pNode );
	};
	AwardRecord m_PerDifficultyAwards[NUM_STEPS_TYPES][NUM_DIFFICULTIES][NUM_PER_DIFFICULTY_AWARDS];
	AwardRecord m_PeakComboAwards[NUM_PEAK_COMBO_AWARDS];
	void AddPerDifficultyAward( StepsType st, Difficulty dc, PerDifficultyAward pda );
	void AddPeakComboAward( PeakComboAward pca );
	bool HasPerDifficultyAward( StepsType st, Difficulty dc, PerDifficultyAward pda );
	bool HasPeakComboAward( PeakComboAward pca );

	//
	// RecentSongScores
	//
	struct HighScoreForASongAndSteps
	{
		StepsID stepsID;
		SongID songID;
		HighScore hs;

		HighScoreForASongAndSteps() { Unset(); }
		void Unset() { stepsID.Unset(); songID.Unset(); hs.Unset(); }

		XNode* CreateNode() const;
		void LoadFromNode( const XNode* pNode );
	};
	vector<HighScoreForASongAndSteps> m_vRecentStepsScores;
	void AddStepsRecentScore( const Song* pSong, const Steps* pSteps, HighScore hs );
	
	//
	// RecentCourseScores
	//
	struct HighScoreForACourse
	{
		CourseID courseID;
		TrailID	trailID;
		HighScore hs;

		HighScoreForACourse() { Unset(); }
		void Unset() { courseID.Unset(); hs.Unset(); }

		XNode* CreateNode() const;
		void LoadFromNode( const XNode* pNode );
	};
	vector<HighScoreForACourse> m_vRecentCourseScores;
	void AddCourseRecentScore( const Course* pCourse, const Trail* pTrail, HighScore hs );

	//
	// Init'ing
	//
	void InitAll()
	{
		InitEditableData(); 
		InitGeneralData(); 
		InitSongScores(); 
		InitCourseScores(); 
		InitCategoryScores(); 
		InitScreenshotData(); 
		InitCalorieData(); 
		InitAwards(); 
		InitRecentSongScores(); 
		InitRecentCourseScores(); 
	}
	void InitEditableData(); 
	void InitGeneralData(); 
	void InitSongScores(); 
	void InitCourseScores(); 
	void InitCategoryScores(); 
	void InitScreenshotData(); 
	void InitCalorieData(); 
	void InitAwards(); 
	void InitRecentSongScores(); 
	void InitRecentCourseScores(); 

	//
	// Loading and saving
	//
	bool LoadAllFromDir( CString sDir, bool bRequireSignature );	// return false on error
	bool SaveAllToDir( CString sDir, bool bSignData ) const;

	void LoadEditableDataFromDir( CString sDir );
	void LoadGeneralDataFromNode( const XNode* pNode );
	void LoadSongScoresFromNode( const XNode* pNode );
	void LoadCourseScoresFromNode( const XNode* pNode );
	void LoadCategoryScoresFromNode( const XNode* pNode );
	void LoadScreenshotDataFromNode( const XNode* pNode );
	void LoadCalorieDataFromNode( const XNode* pNode );
	void LoadAwardsFromNode( const XNode* pNode );
	void LoadRecentSongScoresFromNode( const XNode* pNode );
	void LoadRecentCourseScoresFromNode( const XNode* pNode );

	void SaveEditableDataToDir( CString sDir ) const;
	XNode* SaveGeneralDataCreateNode() const;
	XNode* SaveSongScoresCreateNode() const;
	XNode* SaveCourseScoresCreateNode() const;
	XNode* SaveCategoryScoresCreateNode() const;
	XNode* SaveScreenshotDataCreateNode() const;
	XNode* SaveCalorieDataCreateNode() const;
	XNode* SaveAwardsCreateNode() const;
	XNode* SaveRecentSongScoresCreateNode() const;
	XNode* SaveRecentCourseScoresCreateNode() const;

	void SaveStatsWebPageToDir( CString sDir ) const;
	void SaveMachinePublicKeyToDir( CString sDir ) const;

private:
	const HighScoresForASong *GetHighScoresForASong( const SongID& songID ) const;
	const HighScoresForACourse *GetHighScoresForACourse( const CourseID& courseID ) const;
};


#endif

/*
 * (c) 2001-2004 Chris Danford
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
