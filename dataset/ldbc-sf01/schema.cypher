create node table Comment (id int64, creationDate INT64, locationIP STRING, browserUsed STRING, content STRING, length INT32, PRIMARY KEY (id));
create node table Forum (id INT64, title STRING, creationDate INT64, PRIMARY KEY (id));
create node table Organisation (id INT64, label STRING, name STRING, url STRING, PRIMARY KEY (id));
create node table Person (id INT64, firstName STRING, lastName STRING, gender STRING, birthday INT64, creationDate INT64, locationIP STRING, browserUsed STRING, PRIMARY KEY (id));
create node table Place (id INT64, name STRING, url STRING, label STRING, PRIMARY KEY (id));
create node table Post (id INT64, imageFile STRING, creationDate INT64, locationIP STRING, browserUsed STRING, language STRING, content STRING, length INT32, PRIMARY KEY (id));
create node table Tag (id INT64, name STRING, url STRING, PRIMARY KEY (id));
create node table TagClass (id INT64, name STRING, url STRING, PRIMARY KEY (id));
create rel table Comment_hasCreator_Person (FROM Comment TO Person, MANY_MANY);
create rel table Comment_hasTag_Tag (FROM Comment TO Tag, MANY_MANY);
create rel table Comment_isLocatedIn_Place (FROM Comment TO Place, MANY_ONE);
create rel table Comment_replyOf_Comment (FROM Comment TO Comment, MANY_ONE);
create rel table Comment_replyOf_Post (FROM Comment TO Post, MANY_ONE);
create rel table Forum_containerOf_Post (FROM Forum TO Post, ONE_MANY);
create rel table Forum_hasMember_Person (FROM Forum TO Person, joinDate INT64, MANY_MANY);
create rel table Forum_hasModerator_Person (FROM Forum TO Person, MANY_MANY);
create rel table Forum_hasTag_Tag (FROM Forum TO Tag, MANY_MANY);
create rel table Organisation_isLocatedIn_Place (FROM Organisation TO Place, MANY_ONE);
create rel table Person_hasInterest_Tag (FROM Person TO Tag, MANY_MANY);
create rel table Person_isLocatedIn_Place (FROM Person TO Place, MANY_ONE);
create rel table Person_knows_Person (FROM Person TO Person, creationDate INT64, MANY_MANY);
create rel table Person_likes_Comment (FROM Person TO Comment, creationDate INT64, MANY_MANY);
create rel table Person_likes_Post (FROM Person TO Post, creationDate INT64, MANY_MANY);
create rel table Person_studyAt_Organisation (FROM Person TO Organisation, classYear INT32, MANY_ONE);
create rel table Person_workAt_Organisation (FROM Person TO Organisation, workFrom INT32, MANY_MANY);
create rel table Place_isPartOf_Place (FROM Place TO Place, MANY_ONE);
create rel table Post_hasCreator_Person (FROM Post TO Person, MANY_ONE);
create rel table Post_hasTag_Tag (FROM Post TO Tag, MANY_MANY);
create rel table Post_isLocatedIn_Place (FROM Post TO Place, MANY_ONE);
create rel table Tag_hasType_TagClass (FROM Tag TO TagClass, MANY_ONE);
create rel table TagClass_isSubclassOf_TagClass (FROM TagClass TO TagClass, MANY_ONE);