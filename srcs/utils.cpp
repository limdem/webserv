#include "../includes/webserv.hpp"

std::vector<std::string>		split2(const std::string& str, char c)
{
	std::vector<std::string> tokens;
	std::string token;
	//std::istringstream tokenStream(str);


    if (str == "\0")
    {
        tokens.push_back("\n");
        return tokens;
    }

	std::istringstream tokenStream(str);	

	while (std::getline(tokenStream, token, c))
		tokens.push_back(token);
	return tokens;
}

std::vector<std::string>		split(const std::string& str)
{
	std::vector<std::string> tokens;
	std::string token;
	int	i = 0;
	int	debut = 0;
	int	fin = 0;
	int	length = 0;


    if (str == "\0" || str == "\n")
    {
        tokens.push_back("\n");
        return tokens;
    }

	

	while (str[i] != '\0')
	{
		while (str[i] == ' ' || str[i] == '\t')
			i++;
		debut = i;
		while (str[i] != ' ' && str[i] != '\t' && str[i] != '\0')
			i++;
		fin = i;
		length = fin - debut; 
		token = str.substr(debut, length);
		tokens.push_back(token);
	}
	return tokens;
}

int is_closed_chevron(std::string str)
{
	int	i = 0;

	if (str == "\0")
		return (0);

	while (str[i] == ' ' || str[i] == '\t')
		i++;
	if (str[i] == '}' && str[i+1] == '\0')
		return 1;
	else
		return 0;
}

int end_equality(std::string name, std::string requete_name)
{
	int i = 0;
	int requete_name_size = requete_name.size();
	int	name_size = name.size();

	while (i < requete_name_size && i < name_size)
	{
		if (requete_name[requete_name_size -1 - i] == name[name_size - 1 - i])
			i++;
		else if (name[name_size -1 - i] == '*' && i == name_size - 1)
			return name_size;
		else
			return (0);
	}
	return (0);

}

int begin_equality(std::string name, std::string requete_name)
{
	int i = 0;
	int requete_name_size = requete_name.size();
	int	name_size = name.size();

	while (i < requete_name_size && i < name_size)
	{
		if (requete_name[i] == name[i])
			i++;
		else if (name[i] == '*' && i == name_size - 1)
			return name_size;
		else
			return (0);
	}
	return (0);

}

int		check_extension(std::string s)
{
	std::string::difference_type n = std::count(s.begin(), s.end(), '.');
	if (n > 1)
		return 0;
	return 1;
}

int		check_good_extension(std::string s)
{
		std::string::size_type n = s.size();

	if (n < 4)
		return 0;
	if (s[n-1] == 'p' && s[n-2] == 'h' && s[n-3] == 'p' && s[n-4] == '.')
		return 1;
	if (s[n-1] == 'y' && s[n-2] == 'p' && s[n-3] == '.')
		return 2;
	if (s[n-1] == 'l' && s[n-2] == 'p' && s[n-3] == '.')
		return 3;
	return 0;
}

std::string			get_Link(std::string const &dirEntry, std::string const &dirName, std::string const &host, int port)
{
    std::stringstream   ss;
	
    ss << "\t\t<p><a href=\"http://" + host + ":" <<\
        port <<  dirName  + "/" + dirEntry + "\">"  + dirEntry + "</a></p>\n";
    return ss.str();
}

int		check_file(char *str)
{
	int i = 0;
	int tmp = 0;
	while (str[i] != '\0')
	{
		if (str[i] == '.')
		{
			tmp = 1;
			break;
		}
		i++;
	}
	if (tmp)
		return (1);
	else
		return (0);
}
















