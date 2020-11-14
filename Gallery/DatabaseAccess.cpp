#include "DatabaseAccess.h"

/*
callback for return albums 
*/
int callbackGetAlbums(void* data, int argc, char** argv, char** azColName)
{
	std::list<Album>* listAlbums = (std::list<Album>*)data;
	Album temp(0,"");
	for (int i = 0; i < argc; i++) {
		if (std::string(azColName[i]) == "NAME") {
			temp.setName(argv[i]);
		}
		else if (std::string(azColName[i]) == "CREATION_DATE") {
			temp.setCreationDate(argv[i]);
		}
		else if (std::string(azColName[i]) == "USER_ID") {
			temp.setOwner(std::stoi(argv[i]));
			listAlbums->push_back(temp);
		}
	}
	return 0;
}

/*
callback for return users
*/
int callbackGetUser(void* data, int argc, char** argv, char** azColName)
{
	std::list<User>* listUsers = (std::list<User>*)data;
	User r(0,"");
	for (int i = 0; i < argc; i++) {
		if (std::string(azColName[i]) == "ID") {
			r.setId(atoi(argv[i]));
		}
		else if (std::string(azColName[i]) == "NAME") {
			r.setName(argv[i]);
			listUsers->push_back(r);
		}
	}
	return 0;
}

/*
callback for return pictures
*/
int callbackGetPictures(void* data, int argc, char** argv, char** azColName)
{
	std::list<Picture>* listPictures = (std::list<Picture>*)data;
	Picture temp(0,"");
	//ID NAME LOCATION CREATION_DATE ALBUM_ID 
	for (int i = 0; i < argc; i++) 
	{
		if (std::string(azColName[i]) == "ID") {
			temp.setId(std::stoi(argv[i]));
		}
		else if (std::string(azColName[i]) == "NAME") {	
			temp.setName(argv[i]);
		}
		else if (std::string(azColName[i]) == "LOCATION") {
			temp.setPath(argv[i]);
		}
		else if (std::string(azColName[i]) == "CREATION_DATE") {
			temp.setCreationDate(argv[i]);
		}
		else if (std::string(azColName[i]) == "ALBUM_ID") {
			listPictures->push_back(temp);
		}
	}
	return 0;
}

/*
callback for return tags
*/
int callbackGetTags(void* data, int argc, char** argv, char** azColName)
{
	std::list<Tag>* listTags = (std::list<Tag>*)data;
	Tag temp(0,0,0);
	for (int i = 0; i < argc; i++)
	{
		if (std::string(azColName[i]) == "ID") {
			temp.setID(std::stoi(argv[i]));
		}
		else if (std::string(azColName[i]) == "PICTURE_ID") {
			temp.setPictureID(std::stoi(argv[i]));
		}
		else if (std::string(azColName[i]) == "USER_ID") {
			temp.setUserID(std::stoi(argv[i]));
			listTags->push_back(temp);
		}
	}
	return 0;
}

/*
callback for print answer from DB
*/
int callbackPrint(void* data, int argc, char** argv, char** azColName)
{
	for (int i = 0; i < argc; i++)
	{
		std::cout << azColName[i] << " = " << argv[i] << "";
		if (i == argc - 1)
		{
			std::cout << "";
		}
		else
		{
			std::cout << ", ";
		}
	}
	std::cout << std::endl;
	return 0;
}

/*
callback for check if exsits in DB
*/
int callbackexist(void* data, int argc, char** argv, char** azColName)
{
	bool* exsits = (bool*)data;
	*exsits = argc > 0;
	return 0;
}

int callbackGetCount(void* data, int argc, char** argv, char** azColName)
{
	int* count = (int*)data;
	*count = argc;
	return 0;
}


/*
Open the DB and if not exists it creates it
*/
bool DatabaseAccess::open()
{
	std::string dbFileName = "GalleryDB.sqlite";
	int doesFileExist = _access(dbFileName.c_str(), 0);
	int res = sqlite3_open(dbFileName.c_str(), &_db);
	if (res != SQLITE_OK) {
		_db = nullptr;
		std::cout << "Failed to open DB" << std::endl;
		return -1;
	}
	if (doesFileExist == 0 || doesFileExist == -1) {
		char sqlStatementCreateUsers[] = "CREATE TABLE Users( ID integer PRIMARY KEY AUTOINCREMENT NOT NULL, NAME text NOT NULL);";
		char sqlStatementCreateAlbums[] = "CREATE TABLE Albums( ID integer PRIMARY KEY AUTOINCREMENT NOT NULL, NAME text NOT NULL, CREATION_DATE text NOT NULL, USER_ID integer NOT NULL, FOREIGN KEY(USER_ID) REFERENCES Users(ID)); ";
		char sqlStatementCreatePictures[] = "CREATE TABLE Pictures( ID integer PRIMARY KEY AUTOINCREMENT NOT NULL, NAME text NOT NULL, LOCATION text NOT NULL, CREATION_DATE text NOT NULL, ALBUM_ID integer NOT NULL, FOREIGN KEY(ALBUM_ID) REFERENCES Albums(ID)); ";
		char sqlStatementCreateTags[] = "CREATE TABLE Tags( ID integer PRIMARY KEY AUTOINCREMENT NOT NULL, PICTURE_ID integer NOT NULL, USER_ID integer NOT NULL, FOREIGN KEY(PICTURE_ID) REFERENCES Pictures(ID), FOREIGN KEY(USER_ID) REFERENCES Users(ID)); ";
		char* errMessage = nullptr;
		res = sqlite3_exec(_db, sqlStatementCreateUsers, nullptr, nullptr, &errMessage);
		if (res != SQLITE_OK)
			return false;

		res = sqlite3_exec(_db, sqlStatementCreateAlbums, nullptr, nullptr, &errMessage);
		if (res != SQLITE_OK)
			return false;

		res = sqlite3_exec(_db, sqlStatementCreatePictures, nullptr, nullptr, &errMessage);
		if (res != SQLITE_OK)
			return false;

		res = sqlite3_exec(_db, sqlStatementCreateTags, nullptr, nullptr, &errMessage);
		if (res != SQLITE_OK)
			return false;
	}

	return true;
}

/*
Close the DB
*/
void DatabaseAccess::close()
{
	sqlite3_close(_db);
	_db = nullptr;
}

void DatabaseAccess::clear(){}

/*
The methos is building a list of albums
Get all albums and pictures in album and tags
*/
const std::list<Album> DatabaseAccess::getAlbums()
{
	char* err;
	int res;
	std::list<Album> lst;
	sqlite3_exec(_db, "SELECT * FROM Albums;", callbackGetAlbums, &lst, nullptr);//get all albums
	std::list<Picture> l;
	for (auto it = lst.begin(); it != lst.end(); it++)
	{
		std::list<Tag> t;
		l = std::list<Picture>();
		//get all pictures in album
		std::string quary = "SELECT * FROM Pictures WHERE Pictures.ALBUM_ID = (SELECT Albums.ID FROM Albums WHERE Albums.USER_ID = " + std::to_string(it->getOwnerId()) + " AND Albums.NAME = \"" + it->getName() + "\");";
		res = sqlite3_exec(_db, quary.c_str(), callbackGetPictures, &l, &err);
		if(res)
		{
			std::cout << err << std::endl;
		}
		
		for (auto pic : l)
		{
			t = std::list<Tag>();
			//get all tags in picture 
			std::string quary2 = "SELECT * FROM Tags WHERE Tags.PICTURE_ID = " + std::to_string(pic.getId()) + ";";
			res = sqlite3_exec(_db, quary2.c_str(), callbackGetTags, &t, &err);
			if (res)
			{
				std::cout << err << std::endl;
			}
			for (auto tag : t)
			{
				pic.tagUser(tag.getUserID());//tag user in picture
			}
			it->addPicture(pic);//add the picture to album
		}
	}
	return lst;
}

/*
Get all albums of user
*/
const std::list<Album> DatabaseAccess::getAlbumsOfUser(const User& user)
{
	std::list<Album> allAlbums = getAlbums();
	std::list<Album> userAlbums;
	for (auto it = allAlbums.begin(); it != allAlbums.end(); it++)
	{
		if (it->getOwnerId() == user.getId())//check if it's the user album
		{
			userAlbums.push_back(*it);
		}
	}
	return userAlbums;

}

/*
Creates album and insert it to DB
*/
void DatabaseAccess::createAlbum(const Album& album)
{
	int res;
	char* errMessage = nullptr;
	std::string insertAlbumSql = "INSERT INTO Albums(NAME, CREATION_DATE, USER_ID) VALUES( \"" + album.getName() + "\", \"" + album.getCreationDate() + "\", " + std::to_string(album.getOwnerId()) + ");";
	res = sqlite3_exec(_db, insertAlbumSql.c_str(), nullptr, nullptr, &errMessage);
	if (res != SQLITE_OK)
	{
		std::cout << errMessage << std::endl;
		return;
	}
}

/*
Delete album from DB and it's picturs
*/
void DatabaseAccess::deleteAlbum(const std::string& albumName, int userId)
{
	int res;
	char* errMessage = nullptr;
	std::list<Album> lst;
	std::string albumSql = "SELECT * FROM Albums WHERE NAME = \"" + albumName + "\";";
	res = sqlite3_exec(_db, albumSql.c_str(), callbackGetAlbums, &lst, nullptr);
	if (res)
	{
		std::cout << errMessage << std::endl;
		return;
	}
	
	std::list<Picture> l;
	std::string quary = "SELECT * FROM Pictures WHERE Pictures.ALBUM_ID = (SELECT Albums.ID FROM Albums WHERE Albums.USER_ID = " + std::to_string(lst.front().getOwnerId()) + " AND Albums.NAME = \"" + lst.front().getName() + "\");";
	res = sqlite3_exec(_db, quary.c_str(), callbackGetPictures, &l, &errMessage);
	if (res)
	{
		std::cout << errMessage << std::endl;
		return;
	}
	for (auto pic : l)
	{
		removePictureFromAlbumByName(albumName, pic.getName());
	}


	std::string deleteAlbumSql = "DELETE FROM Albums WHERE NAME == \"" + albumName + "\";";
	res = sqlite3_exec(_db, deleteAlbumSql.c_str(), nullptr, nullptr, &errMessage);
	if (res != SQLITE_OK)
	{
		std::cout << errMessage << std::endl;
		return;
	}
}

/*
return count of album that the user owned
*/
int DatabaseAccess::countAlbumsOwnedOfUser(const User& user)
{	
	return getAlbumsOfUser(user).size();
}

/*
Return the count of album that user taged in
*/
int DatabaseAccess::countAlbumsTaggedOfUser(const User& user)
{
	int res;
	char* err;
	std::list<Tag>tags;
	std::string countTagAlbumsSQL = "SELECT * FROM Tags INNER JOIN Users ON Tags.USER_ID = Users.ID WHERE Tags.USER_ID = " + std::to_string(user.getId()) + ";";
	res = sqlite3_exec(_db, countTagAlbumsSQL.c_str(), callbackGetTags, &tags, &err);
	if (res)
	{
		std::cout << err << std::endl;
		return -1;
	}
	return tags.size();
}

/*
Check where the user is taged and return how much
*/
int DatabaseAccess::countTagsOfUser(const User& user)
{
	std::list<Tag>tags;
	std::string tagCount = "SELECT * FROM Tags WHERE Tags.USER_ID = " + std::to_string(user.getId()) + ";";
	sqlite3_exec(_db,tagCount.c_str() , callbackGetTags, &tags, nullptr);
	return tags.size();
}

/*
Checks if an album exists
*/
bool DatabaseAccess::doesAlbumExists(const std::string& albumName, int userId)
{
	bool exsits = false;
	char* err;
	int res = 0;
	std::cout << "Album name: " << albumName << std::endl << "---------------\n";
	std::string exsitsSQL = "SELECT * FROM Albums WHERE Albums.USER_ID = " + std::to_string(userId) + " AND Albums.NAME LIKE \"" + albumName + "\";";
	res = sqlite3_exec(_db, exsitsSQL.c_str(), callbackexist, &exsits, &err);
	if (res) {
		std::cout << err << std::endl;
	}
	return exsits;
}

/*
Check if user exists
*/
bool DatabaseAccess::doesUserExists(int userId)
{
	bool exsits = false;
	std::string sql = "SELECT ID FROM Users WHERE Users.ID = " + std::to_string(userId) + ";";
	sqlite3_exec(_db, sql.c_str(), callbackexist, &exsits, nullptr);
	return exsits;
}

/*
Print all albums
*/
void DatabaseAccess::printAlbums()
{
	sqlite3_exec(_db, "SELECT * FROM Albums;", callbackPrint, nullptr, nullptr);
}

/*
Print all users
*/
void DatabaseAccess::printUsers()
{
	sqlite3_exec(_db, "SELECT * FROM Users;", callbackPrint, nullptr, nullptr);
}

/*
Get user by it's ID
*/
User DatabaseAccess::getUser(int userId)
{
	std::list<User> lst;
	std::string sql = "SELECT * FROM Users WHERE Users.ID = " + std::to_string(userId) + ";";
	sqlite3_exec(_db, sql.c_str(), callbackGetUser, &lst, nullptr);
	return lst.front();
}

/*
Open album by it's name and return it
*/
Album DatabaseAccess::openAlbum(const std::string& albumName)
{
	std::list<Album> aList = getAlbums();//get all albums
	for (auto it = aList.begin(); it != aList.end(); it++)
	{
		if (it->getName() == albumName)//if the name is equal so return the right album
		{
			return *it;
		}
	}
	return Album();

}

void DatabaseAccess::closeAlbum(Album& pAlbum) {}

/*
Add picutre to DB
*/
void DatabaseAccess::addPictureToAlbumByName(const std::string& albumName, const Picture& picture)
{
	int res;
	char* err;
	std::string sql = "INSERT INTO Pictures(NAME,LOCATION,CREATION_DATE,ALBUM_ID) VALUES(\"" + picture.getName()
		+ "\", \"" + picture.getPath() + "\", \"" + picture.getCreationDate() + "\", ";
	sql += "(SELECT ID FROM Albums WHERE NAME LIKE \"" + albumName + "\"));";
	res = sqlite3_exec(_db, sql.c_str(), nullptr, nullptr, &err);
	if (res != SQLITE_OK)
	{
		std::cout << err << std::endl;
		return;
	}
}

/*
Remove picture from DB and from album object
*/
void DatabaseAccess::removePictureFromAlbumByName(const std::string& albumName, const std::string& pictureName)
{
	int res;
	char* err;

	std::list<Picture> lst;
	std::string getPicSql = "SELECT * FROM Pictures WHERE NAME = \"" + pictureName + "\" AND album_id = " + "(SELECT ID FROM Albums WHERE NAME = \"" + albumName + "\");";
	res = sqlite3_exec(_db, getPicSql.c_str(), callbackGetPictures, &lst, &err);
	if (res != SQLITE_OK)
	{
		std::cout << err << std::endl;
		return;
	}

	Picture pic = lst.front();

	std::list<Tag> t;
	std::string quary2 = "SELECT * FROM Tags WHERE Tags.PICTURE_ID = " + std::to_string(pic.getId()) + ";";
	res = sqlite3_exec(_db, quary2.c_str(), callbackGetTags, &t, &err);
	if (res)
	{
		std::cout << err << std::endl;
		return;
	}
	for (auto tag : t)
	{
		std::string deleteTagSql = "DELETE FROM Tags WHERE ID = " + std::to_string(tag.getID()) +";";
		res = sqlite3_exec(_db, deleteTagSql.c_str(), nullptr, nullptr, &err);
		if (res != SQLITE_OK)
		{
			std::cout << err << std::endl;
			return;
		}
		pic.untagUser(tag.getUserID());
	}

	std::string deletePictureSql = "DELETE FROM Pictures WHERE NAME = \"" + pictureName + "\" AND album_id = " + "(SELECT ID FROM Albums WHERE NAME = \"" + albumName + "\");";
	res = sqlite3_exec(_db, deletePictureSql.c_str(), nullptr, nullptr, &err);
	if (res != SQLITE_OK)
	{
		std::cout << err << std::endl;
		return;
	}
}

/*
Tag user in picture
*/
void DatabaseAccess::tagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId)
{
	int res;
	char* err;
	std::string addTagSql = "INSERT INTO Tags(PICTURE_ID, USER_ID) VALUES ((SELECT Pictures.ID FROM Pictures WHERE Pictures.NAME = \"" + pictureName + "\" AND Pictures.ALBUM_ID = (SELECT ID FROM Albums WHERE Albums.NAME = \"" + albumName + "\")), " +  std::to_string(userId) + ");";
	res = sqlite3_exec(_db, addTagSql.c_str(), nullptr, nullptr, &err);
	if (res != SQLITE_OK)
	{
		std::cout << err << std::endl;
		return;
	}
}

/*
Untag user in picture
*/
void DatabaseAccess::untagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId)
{
	int res;
	char* err;
	std::string quary = "DELETE FROM Tags WHERE Tags.PICTURE_ID = (SELECT Pictures.ID FROM Pictures WHERE Pictures.NAME = \"" + pictureName + "\" AND Pictures.ALBUM_ID = (SELECT ID FROM Albums WHERE Albums.NAME = \"" + albumName + "\")) AND USER_ID = " + std::to_string(userId) + ";";
	std::cout << quary << std::endl;
	res = sqlite3_exec(_db, quary.c_str(), nullptr, nullptr, &err);
	if (res)
	{
		std::cout << err << std::endl;
		return;
	}

}

/*
Create user and add it to DB
*/
void DatabaseAccess::createUser(User& user)
{
	int res, count = 0;
	char* err;
	std::string countUser = "SELECT * from Users;";
	res = sqlite3_exec(_db, countUser.c_str(), callbackGetCount, &count, &err);
	if (res != SQLITE_OK)
	{
		std::cout << err << std::endl;
		return;
	}

	if (count == 0)
	{
		std::string insertUserSql = "INSERT INTO Users(ID, NAME) VALUES( " + std::to_string(user.getId()) + ", \"" + user.getName() + "\");";
		res = sqlite3_exec(_db, insertUserSql.c_str(), nullptr, nullptr, &err);
		if (res != SQLITE_OK)
		{
			std::cout << err << std::endl;
			return;
		}
	}
	else
	{
		std::list<User> lst;

		std::string insertUserSql = "INSERT INTO Users(NAME) VALUES(\"" + user.getName() + "\");";
		res = sqlite3_exec(_db, insertUserSql.c_str(), nullptr, nullptr, &err);
		if (res != SQLITE_OK)
		{
			std::cout << err << std::endl;
			return;
		}

		std::string countUser = "SELECT * from Users WHERE NAME = \"" + user.getName() + "\";";
		res = sqlite3_exec(_db, countUser.c_str(), callbackGetUser, &lst, &err);
		if (res != SQLITE_OK)
		{
			std::cout << err << std::endl;
			return;
		}
		user.setId(lst.front().getId());
		
	}
}

/*
Delete user from DB
*/
void DatabaseAccess::deleteUser(const User& user)
{
	int res;
	std::list<Album> tempA = getAlbumsOfUser(user);
	for (auto a : tempA)
	{
		deleteAlbum(a.getName(), user.getId());
	}
	std::string deleteUserSql = "DELETE FROM Users WHERE ID == " + std::to_string(user.getId()) + ";";
	res = sqlite3_exec(_db, deleteUserSql.c_str(), nullptr, nullptr, nullptr);
	if (res != SQLITE_OK)
		return;
}

/*
Avarge tags per album of user
*/
float DatabaseAccess::averageTagsPerAlbumOfUser(const User& user)
{
	int albumsTaggedCount = countAlbumsTaggedOfUser(user);

	if (0 == albumsTaggedCount) {
		return 0;
	}

	return static_cast<float>(countTagsOfUser(user)) / albumsTaggedCount;
}

/*
Get the top tagged user
*/
User DatabaseAccess::getTopTaggedUser()
{
	int res;
	char* err;
	std::list<Tag> lst;
	std::string getTopSql = "SELECT *, COUNT(user_id) from Tags GROUP BY user_id ORDER BY COUNT(user_id) DESC LIMIT 1;";
	res = sqlite3_exec(_db, getTopSql.c_str(), callbackGetTags, &lst, &err);
	if (res != SQLITE_OK)
	{
		std::cout << err << std::endl;
	}

	std::list<User> lst2;
	std::string getTopSql2 = "SELECT * FROM Users WHERE ID = " + std::to_string(lst.front().getUserID()) + ";";
	res = sqlite3_exec(_db, getTopSql2.c_str(), callbackGetUser, &lst2, &err);
	if (res != SQLITE_OK)
	{
		std::cout << err << std::endl;
	}
	return lst2.front();
}

/*
Get top tagged picture
*/
Picture DatabaseAccess::getTopTaggedPicture()
{
	int res;
	char* err;
	std::list<Tag> lst;
	std::string getTopSql = "SELECT *, COUNT(PICTURE_ID) from Tags GROUP BY PICTURE_ID ORDER BY COUNT(PICTURE_ID) DESC LIMIT 1;";
	res = sqlite3_exec(_db, getTopSql.c_str(), callbackGetTags, &lst, &err);
	if (res != SQLITE_OK)
	{
		std::cout << err << std::endl;
	}

	std::list<Picture> lst2;
	std::string getTopSql2 = "SELECT * FROM Pictures WHERE ID = " + std::to_string(lst.front().getPictureID()) + ";";
	res = sqlite3_exec(_db, getTopSql2.c_str(), callbackGetPictures, &lst2, &err);
	if (res != SQLITE_OK)
	{
		std::cout << err << std::endl;
	}
	return lst2.front();
}

/*
Get tagged pictures of user
*/
std::list<Picture> DatabaseAccess::getTaggedPicturesOfUser(const User& user)
{
	int res;
	char* err;
	std::list<Picture> lst;
	std::string getTopSql2 = "SELECT * FROM Pictures WHERE ID = (SELECT PICTURE_ID FROM Tags JOIN Users ON Users.ID = Tags.USER_ID);";
	res = sqlite3_exec(_db, getTopSql2.c_str(), callbackGetPictures, &lst, &err);
	if (res != SQLITE_OK)
	{
		std::cout << err << std::endl;
	}
	return lst;
}
