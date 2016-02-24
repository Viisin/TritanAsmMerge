
#if defined( _MSC_VER )
#	ifndef _CRT_SECURE_NO_DEPRECATE
#	define _CRT_SECURE_NO_DEPRECATE ( 1 )
#	endif
#	pragma warning( disable : 4996 )
#endif

#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <vector>

class Module
{
	public:
		Module() ;

	public:
		bool load_project( std::string& project ) ;
		bool output_to( std::string& project ) ;

	private:
		void claer() ;

		void set_module_name( std::string& module_name ) ;
		void add_path( std::string& reference_path ) ;
		void add_export_api(std::string& api) ;
		bool add_file( std::string& file ) ;

		void process_sharevar( std::string& original ) ;
		void process_variable( std::string& original ) ;

	private:
		bool                       loaded ;
		std::string                module_name ;
		std::vector< std::string > reference_path ;
		std::vector< std::string > var_0_127 ;
		std::vector< std::string > var_128_511 ;
		std::vector< std::string > var_512_4095 ;
		std::vector< std::string > share_var ;
		std::vector< std::string > public_label ;
		std::vector< std::string > public_var ;
		std::vector< std::string > external_label ;
		std::string                module_text ;
} ;

void Module::claer()
{
	loaded = false ;
	module_name = "" ;
	reference_path.clear() ;
	var_0_127.clear() ;
	var_128_511.clear() ;
	var_512_4095.clear() ;
	share_var.clear() ;
	public_label.clear() ;
	public_var.clear() ;
	external_label.clear() ;
	module_text = "" ;
}

Module::Module()
{
	claer() ;
}

bool Module::load_project( std::string& project )
{
	claer() ;

	std::ifstream source_file( project ) ;
	if( source_file.is_open() == false )
	{
		std::cerr << "TritanAsmMerge: open(" << project << ") failed." << std::endl ;
		return false ;
	}
	std::string original( ( std::istreambuf_iterator<char>( source_file ) ) ,
						  std::istreambuf_iterator<char>() ) ;
	source_file.close() ;

	std::smatch search_match ;
	std::regex  search_regexp ;
	std::string search_text ;

	search_regexp = "^#(\\w+)\\s(.*)" ;

	search_text = original ;
	while( std::regex_search( search_text , search_match , search_regexp ) )
	{
		std::string parameter = search_match[ 1 ] ;
		std::string value = search_match[ 2 ] ;

		if( stricmp( parameter.c_str() , "module" ) == 0 )
		{
			set_module_name( value ) ;
			std::cout << "Set module name : " << value << "\n" ;
		}
		else if( stricmp( parameter.c_str() , "path" ) == 0 )
		{
			add_path( value ) ;
			std::cout << "Add reference path : " << value << "\n" ;
		}
		else if( stricmp( parameter.c_str() , "export" ) == 0 )
		{
			add_export_api( value ) ;
			std::cout << "Add export api : " << value << "\n" ;
		}
		else if( stricmp( parameter.c_str() , "include" ) == 0 )
		{
			std::cout << "Loading(" << value << ")...\r" ;

			if( add_file( value ) == false )
			{
				std::cerr << "TritanAsmMerge: load(" << value << ") failed." << std::endl ;
				return false ;
			}

			std::cout << "Load(" << value << ") [   Finished   ]." << std::endl ;
		}

		search_text = search_match.suffix().str() ;
	}

	return ( loaded = true ) ;
}

bool Module::output_to( std::string& file )
{
	if( loaded == false )
	{
		std::cerr << "TritanAsmMerge: project unloaded." << std::endl ;
		return false ;
	}

	std::ofstream output_file( file ) ;
	if( output_file.is_open() == false )
	{
		std::cerr << "TritanAsmMerge: open(" << file << ") failed." << std::endl ;
		return false ;
	}

	output_file << "VarRM[0:127] = {\n" ;
	for( auto var : var_0_127 )
	{
		output_file << "\t" << var << "\n" ;
	}
	output_file << "}\n" ;

	output_file << "VarRM[128:511] = {\n" ;
	for( auto var : var_128_511 )
	{
		output_file << "\t" << var << "\n" ;
	}
	output_file << "}\n" ;

	output_file << "VarRM[512:4095] = {\n" ;
	for( auto var : var_512_4095 )
	{
		output_file << "\t" << var << "\n" ;
	}
	output_file << "}\n" ;

	output_file << "ShareVar = {\n" ;
	for( auto var : share_var )
	{
		output_file << "\t" << var << "\n" ;
	}
	output_file << "}\n" ;

	output_file << "Public Label {\n" ;
	for( auto var : public_label )
	{
		output_file << "\t" << var << "\n" ;
	}
	output_file << "}\n" ;

	output_file << "Public VarRM {\n" ;
	for( auto var : public_var )
	{
		output_file << "\t" << var << "\n" ;
	}
	output_file << "}\n" ;

	output_file << "External Label {\n" ;
	for( auto var : external_label )
	{
		output_file << "\t" << var << "\n" ;
	}
	output_file << "}\n" ;

	output_file << "Procedure " << module_name << "\n" ;
	output_file << module_text << "\n" ;
	output_file << "EndProc " << module_name << "\n" ;

	output_file.close() ;

	return true ;
}

void Module::set_module_name( std::string& name )
{
	module_name = name ;
}

void Module::add_path( std::string& path )
{
	reference_path.push_back( path ) ;
}

void Module::add_export_api( std::string& api )
{
	public_label.push_back( api ) ;
}

bool Module::add_file( std::string& file )
{
	std::ifstream source_file( file ) ;

	if( source_file.is_open() == false )
	{
		for( auto& path : reference_path )
		{
			std::string new_file ;

			for( auto& x : path )
			{
				if( x != '\"' )
				{
					new_file += x ;
				}
			}

			if( new_file[ new_file.size() - 1 ] != '\\' )
			{
				new_file += '\\' ;
			}

			for( auto& x : file )
			{
				if( x != '\"' )
				{
					new_file += x ;
				}
			}
			
			source_file.open( new_file ) ;
			if( source_file.is_open() == true )
			{
				break ;
			}
		}
	}

	if( source_file.is_open() == false )
	{
		return false ;
	}

	std::string file_text( ( std::istreambuf_iterator<char>( source_file ) ) ,
						   std::istreambuf_iterator<char>() ) ;
	source_file.close() ;

	process_sharevar( file_text ) ;
	process_variable( file_text ) ;

	module_text.append( "\n" ) ;
	module_text.append( file_text ) ;
	module_text.append( "\n" ) ;

	return true ;
}

void Module::process_sharevar( std::string& original )
{
	std::smatch  match ;
	std::regex   regexp_text ;
	std::string  asm_search_text ;

	regexp_text = "ShareVar\\s*=\\s*[{]\\s*([^}]*)[}]" ;

	asm_search_text = original ;
	while( std::regex_search( asm_search_text , match , regexp_text ) )
	{
		std::smatch sharevar_match ;
		std::regex  sharevar_regexp ;
		std::string sharevar_search_text ;

		sharevar_regexp = "/?/?\\s*\\w+\\s*=\\s*\\w+" ;

		sharevar_search_text = match[ 1 ] ;
		while( std::regex_search( sharevar_search_text , sharevar_match , sharevar_regexp ) )
		{
			if( sharevar_search_text.compare( 0 , 2 , "//" ) != 0 )
				share_var.push_back( sharevar_match[ 0 ] ) ;

			sharevar_search_text = sharevar_match.suffix().str() ;
		}

		asm_search_text = match.suffix().str() ;
	}

	original = std::regex_replace( original , regexp_text , "" ) ;
}

void Module::process_variable( std::string& original )
{
	std::smatch  match ;
	std::regex   regexp_text ;
	std::string  asm_search_text ;

	regexp_text = "VarRM\\[(\\w+):\\w+\\]\\s*=\\s*[{]\\s*([^}]*)[}]" ;

	asm_search_text = original ;
	while( std::regex_search( asm_search_text , match , regexp_text ) )
	{
		std::smatch variable_match ;
		std::regex  variable_regexp ;
		std::string variable_search_text ;
		std::string variable_section ;

		variable_regexp = "/?/?\\s*(\\w+\\[*.*)$" ;

		variable_section = match[ 1 ] ;

		variable_search_text = match[ 2 ] ;
		while( std::regex_search( variable_search_text , variable_match , variable_regexp ) )
		{
			if( variable_search_text.compare( 0 , 2 , "//" ) != 0 )
			{
				if( variable_section.compare( "0" ) == 0 )
				{
					var_0_127.push_back( variable_match[ 1 ] ) ;
				}
				else if( variable_section.compare( "128" ) == 0 )
				{
					var_128_511.push_back( variable_match[ 1 ] ) ;
				}
				else if( variable_section.compare( "512" ) == 0 )
				{
					var_512_4095.push_back( variable_match[ 1 ] ) ;
				}
			}
			variable_search_text = variable_match.suffix().str() ;
		}

		asm_search_text = match.suffix().str() ;
	}

	original = std::regex_replace( original , regexp_text , "" ) ;
}

int main( int argc , char** argv )
{
	const unsigned int text_max_len = 1024 ;

	char output_file[ text_max_len ] ;

	Module module ;

	if( argc != 2 )
	{
		std::cerr << "USAGE: TritanAsmMerge.exe <merge project>" << std::endl ;
		exit( -1 ) ;
	}

	if( module.load_project( std::string( argv[ 1 ] ) ) == false )
	{
		exit( -1 ) ;
	}

	strcpy_s( output_file , text_max_len , argv[ 1 ] ) ;
	for( size_t i = strlen( output_file ) - 1 ; i >= 0 ; --i )
	{
		char* p = &( output_file[ i ] ) ;

		if( *p == '.' )
		{
			strcpy_s( p , text_max_len - ( i + 1 ) , "_merged.ASM" ) ;
			break ;
		}

		if( i == 0 )
		{
			strcpy_s( p + 1 , text_max_len - ( i + 1 ) , "_merged.ASM" ) ;
			break ;
		}
	}

	if( module.output_to( std::string( output_file ) ) == false )
	{
		exit( -1 ) ;
	}

	return 0 ;
}
