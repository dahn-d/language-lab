#include "fileRead.h"
#include <errno.h>



fileRead::fileRead(string fName)
{
	m_stream = fopen(fName.c_str(),"r");

	if (m_stream == NULL) {
		perror("");
        char* e = &fName[0];
		throw FileReadException(e);
	}

	m_EOS = false;
	m_LastWordReturnedEOS=false;
	m_readEOS = false;
	m_readChar = false;
};




fileRead::~fileRead()
{
	fclose(m_stream);
}



string fileRead::readString()
{
	if ( m_readEOS )
		return readStringWithEOS();
	else if ( m_readChar)
	{
		string s;
		char ch = fgetc( m_stream );
		if (ch != EOF)
			s.append(1,ch);;
		return(s);
	}
	return readStringWithoutEOS();
}



string fileRead::readStringWithEOS()
{
	string toReturn;

	if ( m_EOS ) {
		m_EOS = false;
		m_LastWordReturnedEOS = true;
		return(EOS);
	}

	if ( feof(m_stream) != 0 ){
		if ( m_LastWordReturnedEOS == false )
		{
			m_LastWordReturnedEOS = true;
			return(EOS);
		}
		return(toReturn);
	}
	
	char ch = fgetc( m_stream );
	while  ( !((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z')) ) 
	{
		if ( ch == '.' || ch == '!' || ch == '?' || ch == 34 )
		{
			if ( m_LastWordReturnedEOS == false )
			{
				m_LastWordReturnedEOS = true;
				return(EOS);
			}
		}

		if ( feof(m_stream) != 0 )
		{
			if ( m_LastWordReturnedEOS == false )
			{
				m_LastWordReturnedEOS = true;
				return(EOS);
			}
			return(toReturn);
		}
		ch = fgetc( m_stream );
	}

	while  ( ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z')) ) 
	{
		if ( (ch >= 'A' && ch <= 'Z') ) ch = ch + 32; // make capitals lowercase
			toReturn.append(1,ch);

		if ( feof(m_stream) != 0 ){
			m_LastWordReturnedEOS = false;
			return(toReturn);
		}
		ch = fgetc( m_stream );
	}

	if ( ch == '.' || ch == '!' || ch == '?' || ch == 34 )
		m_EOS = true;

	m_LastWordReturnedEOS = false;
	return(toReturn);
}



string fileRead::readStringWithoutEOS()
{
	string toReturn;


	if ( feof(m_stream) != 0 ){
		return(toReturn);
	}
	
	char ch = fgetc( m_stream );
	while  ( !((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z')) ) 
	{

		if ( feof(m_stream) != 0 )
		{
			return(toReturn);
		}
		ch = fgetc( m_stream );
	}

	while  ( ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z')) ) 
	{
		if ( (ch >= 'A' && ch <= 'Z') ) ch = ch + 32; // make capitals lowercase
			toReturn.append(1,ch);

		if ( feof(m_stream) != 0 ){
			return(toReturn);
		}
		ch = fgetc( m_stream );
	}

	return(toReturn);
}




void fileRead::readStringTokens(vector<string> &tokens)
{
    tokens.clear();
	
	string s;
	s.assign(readString()); // read next string from file into s

	while ( s.size() != 0 )
	{
		tokens.push_back(s);
		s.assign(readString());  // read next string from file into s
	}
}


void fileRead::readStringTokensEOS(vector<string> &tokens)
{
	 m_readEOS = true;
	 readStringTokens(tokens);   
}



void fileRead::readCharTokens(vector<string> &tokens)
{
	 m_readChar = true;
	 readStringTokens(tokens);
    
}
