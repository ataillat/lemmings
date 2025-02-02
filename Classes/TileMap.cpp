#include "TileMap.h"

bool TileMap::init()
{
    if (!Node::init())
    {
        return false;
    }

    _keyboardListener = cocos2d::EventListenerKeyboard::create();
    _keyboardListener->onKeyPressed = CC_CALLBACK_2(TileMap::skillActivate, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(_keyboardListener, this);

    // offset for UI
    windowOffset = 120;

    // Charger la carte � tuiles
    loadTileMap();

    // scale tilemap
    // X 4.0
    enlargeTileMap(4.0f);

    gameLoop();

    // Ajouter une m�thode update � la boucle de jeu
    this->scheduleUpdate();

    return true;
}

void TileMap::loadTileMap()
{
    // Chargement de la carte � tuiles � partir d'un fichier .tmx
    _tileMap = TMXTiledMap::create("tiled/level.tmx");

    // get layers
    _groundCollisions = _tileMap->getLayer("Ground");
    _wallCollisions = _tileMap->getLayer("Wall");
    _boxCollisions = _tileMap->getLayer("Obstacle");

    // hide collisions layers
    _groundCollisions->setVisible(false);
    _wallCollisions->setVisible(false);
    _boxCollisions->setVisible(false);

    // set to top position
    _tileMap->setPosition(Vec2(0, windowOffset));

    // ajout de la carte � tuiles � la sc�ne
    this->addChild(_tileMap);
}

void TileMap::enlargeTileMap(float scale)
{
    _tileMap->setScale(scale);
}

void TileMap::createStartPortal()
{
    // Cr�ation de l'objet EndPortal
    _startPortal = StartPortal::create();

    // Obtain the object group named "Objects"
    TMXObjectGroup* objectGroup = _tileMap->getObjectGroup("Objects");

    // Obtain the object named "spawn" from the object group
    ValueMap spawn = objectGroup->getObject("Spawn");

    // Retrieve the x and y coordinates of "spawn"
    xSpawn = spawn["x"].asInt();
    xSpawn = xSpawn * 4;
    ySpawn = spawn["y"].asInt();
    ySpawn = ySpawn * 4;

    // Move the tile map by the necessary distance
    _startPortal->setPosition(Vec2(xSpawn, ySpawn + windowOffset));

    //ajout de StartPortal � la sc�ne
    this->addChild(_startPortal);
}

void TileMap::createEndPortal()
{
    // Cr�ation de l'objet EndPortal
    _endPortal = EndPortal::create();

    // Obtain the object group named "Objects"
    TMXObjectGroup* objectGroup = _tileMap->getObjectGroup("Objects");

    // Obtain the object named "Arrival" from the object group
    ValueMap arrival = objectGroup->getObject("Arrival");

    // Retrieve the x and y coordinates of "arrival"
    xArrival = arrival["x"].asInt();
    xArrival = xArrival * 4;
    yArrival = arrival["y"].asInt();
    yArrival = yArrival * 4;

    // Move the tile map by the necessary distance
    _endPortal->setPosition(Vec2(xArrival , yArrival + windowOffset));

    // ajout de EndPortal � la sc�ne
    this->addChild(_endPortal);
}

void TileMap::createLemmings()
{
    // Cr�ation de l'objet Lemmings
    _lemmings = Lemmings::create();

    // set starting direction
    direction = true;

    // Obtain the object group named "Objects"
    TMXObjectGroup* objectGroup = _tileMap->getObjectGroup("Objects");

    // Obtain the object named "spawn" from the object group
    ValueMap spawn = objectGroup->getObject("Spawn");

    // Retrieve the x and y coordinates of "spawn"
    xLemmings = spawn["x"].asInt();
    xLemmings = xLemmings * 4;
    yLemmings = spawn["y"].asInt();
    yLemmings = yLemmings * 4;

    // Move the tile map by the necessary distance
    _lemmings->setPosition(Vec2(xLemmings , yLemmings + windowOffset));

    // ajout de EndPortal � la sc�ne
    this->addChild(_lemmings);
}

bool TileMap::exit()
{
    if (xLemmings >= xArrival)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void TileMap::createBox()
{
    // Cr�ation de l'objet Box
    _box = Box::create();

    // set state
    destroy = false;

    // Obtain the object group named "Objects"
    TMXObjectGroup* objectGroup = _tileMap->getObjectGroup("Objects");

    // Obtain the object named "Box" from the object group
    ValueMap spawn = objectGroup->getObject("Box");

    // Retrieve the x and y coordinates of "spawn"
    xBox = spawn["x"].asInt();
    xBox = xBox * 4;
    yBox = spawn["y"].asInt();
    yBox = yBox * 4;

    // Move the box map by the necessary distance
    _box->setPosition(Vec2(xBox, yBox + windowOffset));

    // ajout de box � la sc�ne
    this->addChild(_box);
}

void TileMap::createSkills()
{
    // Cr�ation de l'objet SkillsMenu
    _skills = SkillsMenu::create();

    // set position
    _skills->setPosition(Vec2(-900, -480));

    // ajout de EndPortal � la sc�ne
    this->addChild(_skills);
}

void TileMap::gameLoop()
{
    isVisible = true;

    // create the portals
    createStartPortal();
    createEndPortal();

    // create Obstacles
    createBox();

    // create Skills
    createSkills();

    // create a sequence with the actions and callbacks
    auto createLemmingsAction = CallFunc::create([this]() { this->createLemmings(); });

    auto startPortalAppearing = CallFunc::create(CC_CALLBACK_0(StartPortal::appearingPortalAnimation, _startPortal));
    auto startPortalIdle = CallFunc::create(CC_CALLBACK_0(StartPortal::idlePortalAnimation, _startPortal));

    auto endPortalAppearing = CallFunc::create(CC_CALLBACK_0(EndPortal::appearingPortalAnimation, _endPortal));
    auto endPortalIdle = CallFunc::create(CC_CALLBACK_0(EndPortal::idlePortalAnimation, _endPortal));

    auto delayStartingAnimation = DelayTime::create(1.0f);
    auto delayIdleAnimation = DelayTime::create(0.8f);

    // set sequence
    auto seq = Sequence::create
    (
        delayStartingAnimation,
        startPortalAppearing,
        endPortalAppearing,
        delayIdleAnimation,
        startPortalIdle,
        endPortalIdle,
        createLemmingsAction,
        nullptr
    );

    // run it
    runAction(seq);
}

bool TileMap::collideGround()
{
    // deplacement vers le bas
    Vec2 lemmingsPos = _lemmings->getPosition();
    lemmingsPos.x /= 64.0f;
    lemmingsPos.y /= 64.0f;
    lemmingsPos.y = 17.4f - lemmingsPos.y;
    bool tileGid = _groundCollisions->getTileGIDAt(lemmingsPos);
    if (tileGid) {
        // touche le sol
        return true;
    }
}

bool TileMap::collideWall()
{
    // deplacement horizontal
    Vec2 lemmingsPos = _lemmings->getPosition();
    lemmingsPos.x /= 64.0f;
    lemmingsPos.y /= 64.0f;
    bool tileGid = _wallCollisions->getTileGIDAt(lemmingsPos);
    if (tileGid) {
        // touche le mur
        return true;
    }
}

bool TileMap::collideBox()
{
    // deplacement horizontal
    Vec2 lemmingsPos = _lemmings->getPosition();
    lemmingsPos.x /= 64.0f;
    lemmingsPos.y /= 64.0f;
    bool tileGid = _boxCollisions->getTileGIDAt(lemmingsPos);
    if (tileGid) {
        // touch the box
        if (destroy)
        {
            return false;
        }
        return true;
    }
}

// Fonction update qui sera appel�e � chaque frame
void TileMap::update(float delta)
{
    if (_lemmings)
    {
        // recuperation de la position du lemmings
        xLemmings = _lemmings->getPosition().x;
        yLemmings = _lemmings->getPosition().y;

        collideGround();
        collideWall();
        exit();

        // chute
        if (!collideGround() && !exit())
        {
            _lemmings->drop();
        }

        if (collideGround() && !exit())
        {
            if (direction)
            {
                // avance
                _lemmings->advance();
                if (collideWall())
                {
                    direction = false;
                }
                if (collideBox() && isVisible)
                {
                    direction = false;
                }
            }
            else
            {
                // recule
                _lemmings->backOff();
                if (collideWall())
                {
                    direction = true;
                }
                if (collideBox() && isVisible)
                {
                    direction = true;
                }
            }
        }

        // sortie
        if (exit())
        {
            if (yLemmings >= yArrival + windowOffset)
            {
                _lemmings->disappears();
                _lemmings = nullptr;

                if (true) // si tout les lemmings sont pass�s
                {
                    Director::getInstance()->replaceScene(WinScene::createScene());
                }
            }
            else
            {
                _lemmings->output();
            }
        }
    }
}

void TileMap::skillActivate(EventKeyboard::KeyCode keyCode, Event* event)
{
    if (keyCode == EventKeyboard::KeyCode::KEY_UP_ARROW)
    {
        _box->boxDestruction();
        isVisible = false;
    }
}