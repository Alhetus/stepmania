#include "global.h"

#include "ScreenOptionsMaster.h"
#include "RageException.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "ThemeManager.h"
#include "GameState.h"
#include "ScreenManager.h"
#include "NoteSkinManager.h"
#include "Course.h"
#include "Steps.h"
#include "StyleDef.h"
#include "song.h"
#include "SongManager.h"
#include "Character.h"
#include "PrefsManager.h"
#include "ScreenOptionsMasterPrefs.h"
#include "RageSounds.h"
#include "StepMania.h"
#include "RageSoundManager.h"
#include "ProfileManager.h"

#define OPTION_MENU_FLAGS		THEME->GetMetric (m_sName,"OptionMenuFlags")
#define ROW_LINE(i)				THEME->GetMetric (m_sName,ssprintf("Line%i",(i+1)))

#define ENTRY(s)				THEME->GetMetric ("ScreenOptionsMaster",s)
#define ENTRY_NAME(s)			THEME->GetMetric ("OptionNames", s)
#define ENTRY_MODE(s,i)			THEME->GetMetric ("ScreenOptionsMaster",ssprintf("%s,%i",(s).c_str(),(i+1)))
#define ENTRY_DEFAULT(s)		THEME->GetMetric ("ScreenOptionsMaster",(s) + "Default")
#define NEXT_SCREEN				THEME->GetMetric (m_sName,"NextScreen")

// #define NEXT_SCREEN( play_mode )		THEME->GetMetric (m_sName,"NextScreen"+Capitalize(PlayModeToString(play_mode)))
#define PREV_SCREEN				THEME->GetMetric (m_sName,"PrevScreen")
// #define PREV_SCREEN( play_mode )		THEME->GetMetric (m_sName,"PrevScreen"+Capitalize(PlayModeToString(play_mode)))

/* Add the list named "ListName" to the given row/handler. */
void ScreenOptionsMaster::SetList( OptionRowData &row, OptionRowHandler &hand, CString ListName, CString &TitleOut )
{
	hand.type = ROW_LIST;

	TitleOut = ListName;
	if( !ListName.CompareNoCase("noteskins") )
	{
		hand.Default.Init(); /* none */
		row.bOneChoiceForAllPlayers = false;

		CStringArray arraySkinNames;
		NOTESKIN->GetNoteSkinNames( arraySkinNames );
		for( unsigned skin=0; skin<arraySkinNames.size(); skin++ )
		{
			arraySkinNames[skin].MakeUpper();

			ModeChoice mc;
			mc.m_sModifiers = arraySkinNames[skin];
			hand.ListEntries.push_back( mc );
			row.choices.push_back( arraySkinNames[skin] );
		}
		return;
	}

	hand.Default.Load( -1, ENTRY_DEFAULT(ListName) );

	/* Parse the basic configuration metric. */
	CStringArray asParts;
	split( ENTRY(ListName), ",", asParts );
	if( asParts.size() < 1 )
		RageException::Throw( "Parse error in ScreenOptionsMasterEntries::ListName%s", ListName.c_str() );

	row.bOneChoiceForAllPlayers = false;
	const int NumCols = atoi( asParts[0] );
	for( int i=0; i<asParts.size(); i++ )
	{
		if( asParts[i].CompareNoCase("together") == 0 )
			row.bOneChoiceForAllPlayers = true;
		else if( asParts[i].CompareNoCase("multiselect") == 0 )
			row.bMultiSelect = true;
	}

	for( int col = 0; col < NumCols; ++col )
	{
		ModeChoice mc;
		mc.Load( 0, ENTRY_MODE(ListName, col) );
		if( mc.m_sName == "" )
			RageException::Throw( "List \"%s\", col %i has no name", ListName.c_str(), col );

		if( !mc.IsPlayable() )
			continue;

		hand.ListEntries.push_back( mc );

		row.choices.push_back( ENTRY_NAME(mc.m_sName) );
	}
}

/* Add a list of difficulties/edits to the given row/handler. */
void ScreenOptionsMaster::SetStep( OptionRowData &row, OptionRowHandler &hand )
{
	hand.type = ROW_STEP;
	row.bOneChoiceForAllPlayers = false;

	// fill in difficulty names
	if( GAMESTATE->m_bEditing )
	{
		row.choices.push_back( "" );
	}
	else if( GAMESTATE->m_pCurCourse )   // playing a course
	{
		row.bOneChoiceForAllPlayers = true;
		row.choices.push_back( ENTRY_NAME("RegularCourses") );
		if( GAMESTATE->m_pCurCourse->HasDifficult( GAMESTATE->GetCurrentStyleDef()->m_StepsType ) )
			row.choices.push_back( ENTRY_NAME("DifficultCourses") );
	}
	else if( GAMESTATE->m_pCurSong )	// playing a song
	{
		vector<Steps*> vSteps;
		GAMESTATE->m_pCurSong->GetSteps( vSteps, GAMESTATE->GetCurrentStyleDef()->m_StepsType );
		SortNotesArrayByDifficulty( vSteps );
		for( unsigned i=0; i<vSteps.size(); i++ )
		{
			Steps* pSteps = vSteps[i];

			CString s;
			// convert to theme-defined difficulty name
			s = SONGMAN->GetDifficultyThemeName( pSteps->GetDifficulty() );
			s += ssprintf( " (%d)", pSteps->GetMeter() );

			row.choices.push_back( s );
		}
	}
	else
	{
		row.choices.push_back( ENTRY_NAME("N/A") );
	}
}


/* Add the given configuration value to the given row/handler. */
void ScreenOptionsMaster::SetConf( OptionRowData &row, OptionRowHandler &hand, CString param, CString &TitleOut )
{
	/* Configuration values are never per-player. */
	row.bOneChoiceForAllPlayers = true;
	hand.type = ROW_CONFIG;

	hand.opt = ConfOption::Find( param );
	if( hand.opt == NULL )
		RageException::Throw( "Invalid Conf type \"%s\"", param.c_str() );

	hand.opt->MakeOptionsList( row.choices );

	TitleOut = hand.opt->name;
}

/* Add a list of available characters to the given row/handler. */
void ScreenOptionsMaster::SetCharacter( OptionRowData &row, OptionRowHandler &hand )
{
	hand.type = ROW_CHARACTER;
	row.bOneChoiceForAllPlayers = false;
	row.choices.push_back( ENTRY_NAME("Off") );
	vector<Character*> apCharacters;
	GAMESTATE->GetCharacters( apCharacters );
	for( unsigned i=0; i<apCharacters.size(); i++ )
	{
		CString s = apCharacters[i]->m_sName;
		s.MakeUpper();
		row.choices.push_back( s ); 
	}
}

/* Add a list of available characters to the given row/handler. */
void ScreenOptionsMaster::SetSaveToProfile( OptionRowData &row, OptionRowHandler &hand )
{
	hand.type = ROW_SAVE_TO_PROFILE;
	row.bOneChoiceForAllPlayers = false;
	row.choices.push_back( ENTRY_NAME("Don'tSave") );
	row.choices.push_back( ENTRY_NAME("SaveToProfile") );
}

ScreenOptionsMaster::ScreenOptionsMaster( CString sClassName ):
	ScreenOptions( sClassName )
{
	LOG->Trace("ScreenOptionsMaster::ScreenOptionsMaster(%s)", m_sName.c_str() );

	/* If this file doesn't exist, leave the music alone (eg. ScreenPlayerOptions music sample
	 * left over from ScreenSelectMusic).  If you really want to play no music, add a redir
	 * to _silent. */
	CString MusicPath = THEME->GetPathToS( ssprintf("%s music", m_sName.c_str()), true );
	if( MusicPath != "" )
		SOUND->PlayMusic( MusicPath );

	CStringArray Flags;
	split( OPTION_MENU_FLAGS, ";", Flags, true );
	InputMode im = INPUTMODE_INDIVIDUAL;
	bool Explanations = false;
	int NumRows = -1;

	unsigned i;
	for( i = 0; i < Flags.size(); ++i )
	{
		Flags[i].MakeLower();

		if( sscanf( Flags[i], "rows,%i", &NumRows ) == 1 )
			continue;
		if( Flags[i] == "together" )
			im = INPUTMODE_TOGETHER;
		if( Flags[i] == "explanations" )
			Explanations = true;
		if( Flags[i] == "forceallplayers" )
		{
			for( int pn=0; pn<NUM_PLAYERS; pn++ )
				GAMESTATE->m_bSideIsJoined[pn] = true;
			GAMESTATE->m_MasterPlayerNumber = PlayerNumber(0);
		}
		if( Flags[i] == "smnavigation" )
			SetNavigation( NAV_THREE_KEY_MENU );
	}

	if( NumRows == -1 )
		RageException::Throw( "%s::OptionMenuFlags is missing \"rows\" field", m_sName.c_str() );

	m_OptionRowAlloc = new OptionRowData[NumRows];
	for( i = 0; (int) i < NumRows; ++i )
	{
		OptionRowData &row = m_OptionRowAlloc[i];
		
		CString sRowCommands = ROW_LINE(i);
		
		vector<ParsedCommand> vCommands;
		ParseCommands( sRowCommands, vCommands );
		
		if( vCommands.size() < 1 )
			RageException::Throw( "Parse error in %s::Line%i", m_sName.c_str(), i+1 );

		OptionRowHandler hand;
		for( unsigned part = 0; part < vCommands.size(); ++part)
		{
			ParsedCommand& command = vCommands[part];

			HandleParams;

			const CString name = sParam(0);

			if( !name.CompareNoCase("list") )
			{
				SetList( row, hand, sParam(1), row.name );
			}
			else if( !name.CompareNoCase("steps") )
			{
				SetStep( row, hand );
				row.name = "Steps";
			}
			else if( !name.CompareNoCase("conf") )
			{
				SetConf( row, hand, sParam(1), row.name );
			}
			else if( !name.CompareNoCase("characters") )
			{
				SetCharacter( row, hand );
				row.name = "Characters";
			}
			else if( !name.CompareNoCase("SaveToProfile") )
			{
				SetSaveToProfile( row, hand );
				row.name = "Save To\nProfile";
			}
			else
				RageException::Throw( "Unexpected type '%s' in %s::Line%i", name.c_str(), m_sName.c_str(), i );

			CheckHandledParams;
		}
		OptionRowHandlers.push_back( hand );
	}

	ASSERT( (int) OptionRowHandlers.size() == NumRows );

	Init( im, m_OptionRowAlloc, NumRows );
}

ScreenOptionsMaster::~ScreenOptionsMaster()
{
	delete [] m_OptionRowAlloc;
}

int ScreenOptionsMaster::ImportOption( const OptionRowData &row, const OptionRowHandler &hand, int pn, int rowno )
{
	/* Figure out which selection is the default. */
	switch( hand.type )
	{
	case ROW_LIST:
	{
		int ret = -1;
		for( unsigned e = 0; e < hand.ListEntries.size(); ++e )
		{
			const ModeChoice &mc = hand.ListEntries[e];

			if( mc.IsZero() )
			{
				/* The entry has no effect.  This is usually a default "none of the
				 * above" entry.  It will always return true for DescribesCurrentMode().
				 * It's only the selected choice if nothing else matches. */
				ret = e;
				continue;
			}

			if( row.bOneChoiceForAllPlayers )
			{
				if( mc.DescribesCurrentModeForAllPlayers() )
					return e;
			} else {
				if( mc.DescribesCurrentMode( (PlayerNumber) pn) )
					return e;
			}
		}

		if( ret == -1 )
		{
			LOG->Warn( "%s line %i (\"%s\"): couldn't find default", m_sName.c_str(), rowno, row.name.c_str() );
			ret = 0;
		}

		return ret;
	}
	case ROW_STEP:
		if( GAMESTATE->m_bEditing )
			return 0;

		if( GAMESTATE->m_pCurCourse )   // playing a course
		{
			if( GAMESTATE->m_bDifficultCourses &&
				GAMESTATE->m_pCurCourse->HasDifficult( GAMESTATE->GetCurrentStyleDef()->m_StepsType ) )
				return 1;
			return 0;
		}

		if( GAMESTATE->m_pCurSong )	// playing a song
		{
			vector<Steps*> vNotes;
			GAMESTATE->m_pCurSong->GetSteps( vNotes, GAMESTATE->GetCurrentStyleDef()->m_StepsType );
			SortNotesArrayByDifficulty( vNotes );
			for( unsigned i=0; i<vNotes.size(); i++ )
			{
				if( GAMESTATE->m_pCurNotes[pn] == vNotes[i] )
					return i;
			}
		}
		return 0;

	case ROW_CHARACTER:
	{
		vector<Character*> apCharacters;
		GAMESTATE->GetCharacters( apCharacters );
		for( unsigned i=0; i<apCharacters.size(); i++ )
			if( GAMESTATE->m_pCurCharacters[pn] == apCharacters[i] )
				return i+1;
		return 0;
	}

	case ROW_CONFIG:
		return hand.opt->Get( row.choices );

	case ROW_SAVE_TO_PROFILE:
		return 0;

	default:
		ASSERT(0);
		return 0;
	}
}

void ScreenOptionsMaster::ImportOptions()
{
	for( unsigned i = 0; i < OptionRowHandlers.size(); ++i )
	{
		const OptionRowHandler &hand = OptionRowHandlers[i];
		const OptionRowData &data = m_OptionRowAlloc[i];
		Row &row = *m_Rows[i];

		if( data.bOneChoiceForAllPlayers )
		{
			int choice = ImportOption( data, hand, 0, i );
			row.SetOneSharedSelection( choice );
			ASSERT( row.GetOneSharedSelection() < (int)data.choices.size() );
		}
		else
		{
			for( int p=0; p<NUM_PLAYERS; p++ )
			{
				if( !GAMESTATE->IsHumanPlayer(p) )
					continue;

				int choice = ImportOption( data, hand, p, i );
				row.SetOneSelection( (PlayerNumber)p, choice );
				ASSERT( row.GetOneSelection((PlayerNumber)p) < (int)data.choices.size() );
			}
		}
	}
}


/* Returns an OPT mask. */
int ScreenOptionsMaster::ExportOption( const OptionRowData &row, const OptionRowHandler &hand, int pn, int sel )
{
	/* Figure out which selection is the default. */
	switch( hand.type )
	{
	case ROW_LIST:
		hand.Default.Apply( (PlayerNumber)pn );
		hand.ListEntries[sel].Apply( (PlayerNumber)pn );
		break;

	case ROW_CONFIG:
	{
		/* Get the original choice. */
		int Original = hand.opt->Get( row.choices );

		/* Apply. */
		hand.opt->Put( sel, row.choices );

		/* Get the new choice. */
		int New = hand.opt->Get( row.choices );

		/* If it didn't change, don't return any side-effects. */
		if( Original == New )
			return 0;

		return hand.opt->GetEffects();
	}

	case ROW_CHARACTER:
		if( sel == 0 )
			GAMESTATE->m_pCurCharacters[pn] = GAMESTATE->GetDefaultCharacter();
		else
		{
			vector<Character*> apCharacters;
			GAMESTATE->GetCharacters( apCharacters );
			ASSERT( sel - 1 < (int)apCharacters.size() );
			GAMESTATE->m_pCurCharacters[pn] = apCharacters[sel - 1];
		}
		break;

	case ROW_STEP:
		if( GAMESTATE->m_bEditing )
		{
			// do nothing
		}
		else if( GAMESTATE->m_pCurCourse )   // playing a course
		{
			if( sel == 1 )
			{
				GAMESTATE->m_bDifficultCourses = true;
				LOG->Trace("ScreenPlayerOptions: Using difficult course");
			}
			else
			{
				GAMESTATE->m_bDifficultCourses = false;
				LOG->Trace("ScreenPlayerOptions: Using normal course");
			}
		}
		else if( GAMESTATE->m_pCurSong )   // playing a song
		{
			vector<Steps*> vSteps;
			GAMESTATE->m_pCurSong->GetSteps( vSteps, GAMESTATE->GetCurrentStyleDef()->m_StepsType );
			SortNotesArrayByDifficulty( vSteps );
			Steps* pSteps = vSteps[ sel ];
			// set current notes
			GAMESTATE->m_pCurNotes[pn] = pSteps;
			// set preferred difficulty
			GAMESTATE->m_PreferredDifficulty[pn] = pSteps->GetDifficulty();
		}

		break;

	case ROW_SAVE_TO_PROFILE:
		if( sel == 1 )
		{
			if( PROFILEMAN->IsUsingProfile((PlayerNumber)pn) )
			{
				Profile* pProfile = PROFILEMAN->GetProfile((PlayerNumber)pn);
				pProfile->m_bUsingProfileDefaultModifiers = true;
				pProfile->m_sDefaultModifiers = GAMESTATE->m_PlayerOptions[pn].GetString();
			}
		}
		break;

	default:
		ASSERT(0);
		break;
	}
	return 0;
}

void ScreenOptionsMaster::ExportOptions()
{
	int ChangeMask = 0;

	CHECKPOINT;
	unsigned i;
	for( i = 0; i < OptionRowHandlers.size(); ++i )
	{
		const OptionRowHandler &hand = OptionRowHandlers[i];
		const OptionRowData &data = m_OptionRowAlloc[i];
		Row &row = *m_Rows[i];

		for( int p=0; p<NUM_PLAYERS; p++ )
		{
			if( !GAMESTATE->IsHumanPlayer(p) )
				continue;

			ChangeMask |= ExportOption( data, hand, p, row.GetOneSelection((PlayerNumber)p) );
		}
	}

	CHECKPOINT;
	/* If the selection is on a LIST, and the selected LIST option sets the screen,
	 * honor it. */
	m_NextScreen = "";

	const int row = this->GetCurrentRow();
	if( row != -1 )
	{
		const OptionRowHandler &hand = OptionRowHandlers[row];
		if( hand.type == ROW_LIST )
		{
			const int sel = m_Rows[row]->GetOneSharedSelection();
			const ModeChoice &mc = hand.ListEntries[sel];
			if( mc.m_sScreen != "" )
				m_NextScreen = mc.m_sScreen;
		}
	}
	CHECKPOINT;

	// NEXT_SCREEN(GAMESTATE->m_PlayMode) );
	// XXX: handle different destinations based on play mode?
	if( m_NextScreen == "" )
		m_NextScreen = NEXT_SCREEN;

	/* Did the theme change? */
	if( (ChangeMask & OPT_APPLY_THEME) || 
		(ChangeMask & OPT_APPLY_GRAPHICS) ) 	// reset graphics to apply new window title and icon
		ApplyGraphicOptions();

	if( ChangeMask & OPT_SAVE_PREFERENCES )
	{
		/* Save preferences. */
		LOG->Trace("ROW_CONFIG used; saving ...");
		PREFSMAN->SaveGlobalPrefsToDisk();
		SaveGamePrefsToDisk();
	}

	if( ChangeMask & OPT_RESET_GAME )
	{
		ResetGame();
		m_NextScreen = "";
	}

	if( ChangeMask & OPT_APPLY_SOUND )
	{
		SOUNDMAN->SetPrefs( PREFSMAN->m_fSoundVolume );
	}
	
	if( ChangeMask & OPT_SAVE_MODIFIERS_TO_PROFILE )
	{
		SOUNDMAN->SetPrefs( PREFSMAN->m_fSoundVolume );
	}
	CHECKPOINT;
}

void ScreenOptionsMaster::GoToNextState()
{
	if( GAMESTATE->m_bEditing )
		SCREENMAN->PopTopScreen();
	else if( m_NextScreen != "" )
		SCREENMAN->SetNewScreen( m_NextScreen );
}

void ScreenOptionsMaster::GoToPrevState()
{
	/* XXX: A better way to handle this would be to check if we're a pushed screen. */
	if( GAMESTATE->m_bEditing )
		SCREENMAN->PopTopScreen();
	// XXX: handle different destinations based on play mode?
	else
		SCREENMAN->SetNewScreen( PREV_SCREEN ); // (GAMESTATE->m_PlayMode) );
}

void ScreenOptionsMaster::RefreshIcons()
{
	for( int p=0; p<NUM_PLAYERS; p++ )	// foreach player
	{
		if( !GAMESTATE->IsHumanPlayer(p) )
			continue;

        for( unsigned i=0; i<m_Rows.size(); ++i )     // foreach options line
		{
			if( m_Rows[i]->Type == Row::ROW_EXIT )
				continue;	// skip

			Row &row = *m_Rows[i];
			const OptionRowData &data = row.m_RowDef;

			// find first selection and whether multiple are selected
			int iFirstSelection = -1;
			bool bMultipleSelected = false;
			for( int j=0; j<row.m_vbSelected[p].size(); j++ )
			{
				if( row.m_vbSelected[p][j] )
				{
					if( iFirstSelection != -1 )
						bMultipleSelected = true;
					else
						iFirstSelection = j;
				}

			}

			// set icon name
			CString sIcon;

			if( bMultipleSelected )
			{
				sIcon = "Multiple";
			}
			else if( iFirstSelection != -1 )
			{
				const OptionRowHandler &handler = OptionRowHandlers[i];
				switch( handler.type )
				{
				case ROW_LIST:
					sIcon = handler.ListEntries[iFirstSelection].m_sModifiers;
					break;
				case ROW_STEP:
				case ROW_CHARACTER:
					sIcon = data.choices[iFirstSelection];
					break;
				case ROW_CONFIG:
					break;
				}
			}
			

			/* XXX: hack to not display text in the song options menu */
			if( data.bOneChoiceForAllPlayers )
				sIcon = "";

			LoadOptionIcon( (PlayerNumber)p, i, sIcon );
		}
	}
}
