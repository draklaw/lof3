//
//  Copyright (C) 2015 the authors (see AUTHORS)
//
//  This file is part of lof3.
//
//  lair is free software: you can redistribute it and/or modify it
//  under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  lair is distributed in the hope that it will be useful, but
//  WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with lair.  If not, see <http://www.gnu.org/licenses/>.
//


#include <functional>

#include "font.h"
#include "menu.h"
#include "game.h"
#include "fight.h"

#include "main_state.h"


#define ONE_SEC (1000000000)


MainState::MainState(Game* game)
	: _game(game),

      _entities(_game->log()),
      _sprites(_game->renderer()),
      _texts(),
      _inputs(_game->sys(), &_game->log()),

      _slotTracker(),

      _camera(),

      _initialized(false),
      _running(false),
      _loop(_game->sys()),
      _fpsTime(0),
      _fpsCount(0),

      _rules(log()),
      _player{ {0}, {0}, {0,0,0,0,0,0,0,0,0,0,0,0,0}, 0, {0}, {0} },
      _fight(),
      _state(PLAYING),

      _menuInputs(),

      _fontTex(nullptr),
      _fontJson(),
      _font(),

      _music1(nullptr),
      _music2(nullptr),
      _music3(nullptr),
      _transition1(nullptr),
      _transition2(nullptr),

      _bgSprite(),
      _menuBgSprite(),

      _bg(),

      _messages(),
      _messageFrame(),
      _messageMargin(0),

      _menuStack(),
      _mainMenu(),
      _switchMenu(),
      _spellMenu(),
      _summonMenu() {
}


MainState::~MainState() {
}


Sprite MainState::loadSprite(const char* file, unsigned th, unsigned tv) {
	Texture* tex = _game->renderer()->getTexture(
	            file, Texture::NEAREST | Texture::CLAMP);
	return Sprite(tex, th, tv);
}


void MainState::initialize() {
	_loop.reset();
	_loop.setTickDuration(    1000000000 /  60);
	_loop.setFrameDuration(   1000000000 /  60);
	_loop.setMaxFrameDuration(_loop.frameDuration() * 3);
	_loop.setFrameMargin(     _loop.frameDuration() / 2);

	_game->window()->onResize.connect(std::bind(&MainState::layoutScreen, this))
	        .track(_slotTracker);
	layoutScreen();


	_menuInputs.up     = _inputs.addInput("up");
	_menuInputs.down   = _inputs.addInput("down");
//	_menuInputs.left   = _inputs.addInput("left");
//	_menuInputs.right  = _inputs.addInput("right");
	_menuInputs.ok     = _inputs.addInput("ok");
	_menuInputs.cancel = _inputs.addInput("cancel");

	_inputs.mapScanCode(_menuInputs.up,     SDL_SCANCODE_UP);
	_inputs.mapScanCode(_menuInputs.down,   SDL_SCANCODE_DOWN);
//	_inputs.mapScanCode(_menuInputs.left,   SDL_SCANCODE_LEFT);
//	_inputs.mapScanCode(_menuInputs.right,  SDL_SCANCODE_RIGHT);
	_inputs.mapScanCode(_menuInputs.ok,     SDL_SCANCODE_RIGHT);
	_inputs.mapScanCode(_menuInputs.ok,     SDL_SCANCODE_RETURN);
	_inputs.mapScanCode(_menuInputs.ok,     SDL_SCANCODE_SPACE);
	_inputs.mapScanCode(_menuInputs.cancel, SDL_SCANCODE_LEFT);
	_inputs.mapScanCode(_menuInputs.cancel, SDL_SCANCODE_ESCAPE);

	//TODO: Remove cheats.
	_debugInput = _inputs.addInput("Debug");
	_inputs.mapScanCode(_debugInput, SDL_SCANCODE_F1);

	_fontJson = _game->sys()->loader().getJson("8-bit_operator+_regular_23.json");
	_fontTex  = _game->renderer()->getTexture(_fontJson["file"].asString(),
	        Texture::NEAREST | Texture::REPEAT);
	_font.reset(new Font(_fontJson, _fontTex));
	_font->baselineToTop = 12;

	_bgSprite          = loadSprite("bg.png");
	_healthEmptySprite = loadSprite("health_bar_empty.png");
	_healthFullSprite  = loadSprite("health_bar_full.png");
	_menuBgSprite      = loadSprite("menu.png", 3, 3);
	_statusSprite      = loadSprite("status.png", 3, 1);
	_bossSprite[0]     = loadSprite("BigBoss1.png");
	_bossSprite[1]     = loadSprite("BigBoss2.png");
	_bossSprite[2]     = loadSprite("BigBoss3.png");
	_pcSprite[0]       = loadSprite("GTP.png");
	_pcSprite[1]       = loadSprite("MB.png");
	_pcSprite[2]       = loadSprite("MN.png");
	_pcSprite[3]       = loadSprite("Ninja.png");
	_tombSprite        = loadSprite("tomb.png");

	_music1      = _game->audio()->loadMusic(_game->dataPath() / "music1.ogg");
	_music2      = _game->audio()->loadMusic(_game->dataPath() / "music2.ogg");
	_music3      = _game->audio()->loadMusic(_game->dataPath() / "music3.ogg");
	_transition1 = _game->audio()->loadMusic(_game->dataPath() / "transition1.ogg");
	_transition2 = _game->audio()->loadMusic(_game->dataPath() / "transition2.ogg");

	_hitSound       = _game->audio()->loadSound(_game->dataPath() / "hit.ogg");
	_spellSound     = _game->audio()->loadSound(_game->dataPath() / "spell.ogg");
	_healSound      = _game->audio()->loadSound(_game->dataPath() / "heal.ogg");
	_pcDeathSound   = _game->audio()->loadSound(_game->dataPath() / "death_pc.ogg");
	_bossDeathSound = _game->audio()->loadSound(_game->dataPath() / "death_boss.ogg");
	_victorySound   = _game->audio()->loadSound(_game->dataPath() / "boss_victory.ogg");

	_damageAnim.reset(new MoveAnim(ONE_SEC/2, Vector3(0, 30, 0), RELATIVE));
	_damageAnim->onEnd = [this](_Entity* e){ _entities.destroyEntity(EntityRef(e)); };

	_deathAnim.reset(new SwapSpriteAnim(ONE_SEC/10, &_tombSprite, 9));

	_boss0to1Anim.reset(new Sequence);
	_boss0to1Anim->anims.push_back(new MoveAnim(ONE_SEC, Vector3(-300, 0, 0), RELATIVE));
	_boss0to1Anim->anims.push_back(new MoveAnim(ONE_SEC, Vector3(+300, 0, 0), RELATIVE));
	_boss0to1Anim->anims[0]->onEnd = [this](_Entity* e){ e->sprite->setSprite(&_bossSprite[1]); };

	_boss1to2Anim.reset(new Sequence);
	_boss1to2Anim->anims.push_back(new MoveAnim(ONE_SEC, Vector3(-300, 0, 0), RELATIVE));
	_boss1to2Anim->anims.push_back(new MoveAnim(ONE_SEC, Vector3(+280, 0, 0), RELATIVE));
	_boss1to2Anim->anims[0]->onEnd = [this](_Entity* e){ e->sprite->setSprite(&_bossSprite[2]); };


	_statusFrame.reset(new Frame(&_menuBgSprite, Vector2(640, 120)));
	_statusFrame->position = Vector3(0, 0, -.05);

	_messageMargin = 12;
	_messageOutMargin = 8;
	_messageFrame.reset(new Frame(&_menuBgSprite, Vector2(640 - 2 * _messageOutMargin, 0)));
	_messageFrame->position = Vector3(_messageOutMargin, 0, .9);


	_mainMenu.reset(new Menu(&_menuBgSprite, _font.get(), &_menuInputs));
	_switchMenu.reset(new Menu(&_menuBgSprite, _font.get(), &_menuInputs,
	                           closeMenuFunc()));
	_spellMenu.reset(new Menu(&_menuBgSprite, _font.get(), &_menuInputs,
	                          closeMenuFunc()));
	_summonMenu.reset(new Menu(&_menuBgSprite, _font.get(), &_menuInputs,
	                           closeMenuFunc()));
	_pcMenu.reset(new Menu(&_menuBgSprite, _font.get(), &_menuInputs,
	                       closeMenuFunc()));

	_mainMenu->addEntry("Attack", Menu::ENABLED,
	                    openMenuFunc(_pcMenu.get(), _mainMenu.get(), 0));
	_mainMenu->addEntry("Switch", Menu::ENABLED,
	                    openMenuFunc(_switchMenu.get(), _mainMenu.get(), 1));
	_mainMenu->addEntry("Curse", Menu::ENABLED,
	                    openMenuFunc(_spellMenu.get(),  _mainMenu.get(), 2));
//	_mainMenu->addEntry("Summon", Menu::ENABLED,
//	                    openMenuFunc(_summonMenu.get(), _mainMenu.get(), 3));
//	_mainMenu->addEntry("Scan");
//	_mainMenu->addEntry("QTE",     Menu::HIDDEN);
//	_mainMenu->addEntry("Twist 1", Menu::HIDDEN);
	_mainMenu->layout();
	_mainMenu->show(Vector3(0, 0, 0));

	_switchMenu->addEntry("None", Menu::ENABLED, doActionFunc());
	_switchMenu->addEntry("Fire", Menu::ENABLED, doActionFunc());
	_switchMenu->addEntry("Ice", Menu::ENABLED, doActionFunc());
	_switchMenu->addEntry("Thunder", Menu::ENABLED, doActionFunc());
	_switchMenu->addEntry("Acid", Menu::ENABLED, doActionFunc());
	_switchMenu->layout();
	_switchMenu->show(Vector3(_mainMenu->width(), 16, .1));

	_spellMenu->addEntry("Storm", Menu::ENABLED, doActionFunc());
	_spellMenu->addEntry("Strike", Menu::ENABLED,
	                     openMenuFunc(_pcMenu.get(), _spellMenu.get(), 1));
	_spellMenu->addEntry("Crippling strike", Menu::ENABLED,
	                     openMenuFunc(_pcMenu.get(), _spellMenu.get(), 2));
	_spellMenu->addEntry("Soul drain", Menu::ENABLED,
	                     openMenuFunc(_pcMenu.get(), _spellMenu.get(), 3));
	_spellMenu->addEntry("Vorpal sword", Menu::ENABLED,
	                     openMenuFunc(_pcMenu.get(), _spellMenu.get(), 4));
	_spellMenu->addEntry("Mud pit", Menu::ENABLED, doActionFunc());
	_spellMenu->addEntry("Dispel magic", Menu::ENABLED,
	                     openMenuFunc(_pcMenu.get(), _spellMenu.get(), 6));
	_spellMenu->layout();
	_spellMenu->show(Vector3(_mainMenu->width(), 16, .1));

	_summonMenu->addEntry("Sprites", Menu::ENABLED, doActionFunc());
	_summonMenu->addEntry("Tomberry", Menu::ENABLED, doActionFunc());
	_summonMenu->addEntry("Mageling", Menu::ENABLED, doActionFunc());
	_summonMenu->layout();
	_summonMenu->show(Vector3(_mainMenu->width(), 16, .1));

	_pcMenu->addEntry("", Menu::ENABLED, doActionFunc());
	_pcMenu->addEntry("", Menu::ENABLED, doActionFunc());
	_pcMenu->addEntry("", Menu::ENABLED, doActionFunc());
	_pcMenu->addEntry("", Menu::ENABLED, doActionFunc());
	_pcMenu->layout();
	_pcMenu->show(Vector3(_mainMenu->width(), 16, .2));


	_rules.setFromJson(_game->sys()->loader().getJson("rules.json"));

	_initialized = true;
}


void MainState::shutdown() {
	_game->audio()->releaseMusic(_music1);
	_game->audio()->releaseMusic(_music2);
	_game->audio()->releaseMusic(_music3);
	_game->audio()->releaseMusic(_transition1);
	_game->audio()->releaseMusic(_transition2);

	_menuStack.clear();
	_mainMenu.reset();

	_slotTracker.disconnectAll();

	_initialized = false;
}


void MainState::run() {
	lairAssert(_initialized);

	log().log("Starting main state...");
	_running = true;
	_loop.start();
	_fpsTime  = _game->sys()->getTimeNs();
	_fpsCount = 0;

	unsigned lvl = 0;
	do {
		log().log("Our heroes strike",(lvl?".":" again !"));

		lvl += 20;
		_player.strat[rand()%NB_STRATS] = true;
		_player.strat[rand()%NB_STRATS] = true;
		_player.strat[rand()%NB_STRATS] = true;

		initFight(lvl);
		_state = PLAYING;

		do {
			switch(_loop.nextEvent()) {
			case InterpLoop::Tick:
				updateTick();
				break;
			case InterpLoop::Frame:
				updateFrame();
				break;
			}
		} while (_state != GAME_OVER && _running);

	} while (_fight->boss.hp && _running);
	log().log("The mighty has fallen...");

	log().log("Stopping main state...");
	_loop.stop();
}


void MainState::quit() {
	_running = false;
}


void MainState::layoutScreen() {
	int w = _game->window()->width();
	int h = _game->window()->height();
	glViewport(0, 0, w, h);
	_camera.setViewBox(Box3(
		Vector3(  0,           0, -1),
		Vector3(640, 640 * h / w,  1)
	));
}


EntityRef MainState::createSprite(Sprite* sprite, const Vector3& pos,
                                  const char* name) {
	return createSprite(sprite, pos, Vector2(1, 1), name);
}


EntityRef MainState::createSprite(Sprite* sprite, const Vector3& pos,
                                  const Vector2& scale, const char* name) {
	EntityRef entity = _entities.createEntity(_entities.root(), name);
	_sprites.addComponent(entity);
	entity.sprite()->setSprite(sprite);
	entity.place(Translation(pos) * Eigen::Scaling(scale.x(), scale.y(), 1.f));
	_anims.addComponent(entity);
	return entity;
}


EntityRef MainState::createText(const std::string& text, const Vector3& pos,
                                      const Vector4& color) {
	EntityRef entity = _entities.createEntity(_entities.root(), "text");
	_texts.addComponent(entity);
	TextComponent* comp = _texts.get(entity);
	comp->font = _font.get();
	comp->text = text;
	comp->color = color;
	entity.place(Transform(Translation(pos)));
	return entity;
}


EntityRef MainState::createDamageText(const std::string& text, const Vector3& pos,
                                      const Vector4& color) {
	EntityRef entity = createText(text, pos, color);
	_anims.addComponent(entity);
	AnimationComponent* comp = _anims.get(entity);
	comp->anim = _damageAnim->clone();
	return entity;
}


EntityRef MainState::createHealthBar(const Vector3& pos, float size) {
	EntityRef parent = _entities.createEntity(_entities.root());
	parent.place(Transform(Translation(pos)));

	Box2 view(Vector2(0, 0), Vector2(size, 1));
	EntityRef empty = _entities.createEntity(parent, "healthEmpty");
	_sprites.addComponent(empty);
	empty.sprite()->setSprite(&_healthEmptySprite);
	empty.sprite()->setView(view);

	EntityRef full = _entities.createEntity(parent, "healthFull");
	_sprites.addComponent(full);
	full.sprite()->setSprite(&_healthFullSprite);
	full.sprite()->setView(view);
	full.place(Transform(Translation(Vector3(0, 0, 0.01))));

	return full;
}


void MainState::initFight(unsigned lvl) {
	log().log("Initialize main state.");

	while (_entities.root().firstChild().isValid())
		_entities.destroyEntity(_entities.root().firstChild());

	_fight.reset(new Fight(log(), *this, _rules, _player, lvl));

	_damagePos[0] = Vector3(567, _camera.viewBox().max().y() - 120, .5);
	_damagePos[1] = Vector3(517, _camera.viewBox().max().y() - 120, .5);
	_damagePos[2] = Vector3(470, _camera.viewBox().max().y() -  92, .5);
	_damagePos[3] = Vector3(417, _camera.viewBox().max().y() -  78, .5);
	_damagePos[4] = Vector3(120, _camera.viewBox().max().y() -  70, .5);

	_bg = _entities.createEntity(_entities.root(), "bg");
	_sprites.addComponent(_bg);
	_bg.sprite()->setSprite(&_bgSprite);
	_bg.place(Transform(Translation(Vector3(0, _camera.viewBox().max().y() - 480, -.99))));


	_boss = createSprite(&_bossSprite[0],
	                     Vector3(240, _camera.viewBox().max().y() - 240, -.815),
	                     Vector2(-1, 1), "Boss");
//	_boss = createSprite(&_bossSprite[1],
//	                     Vector3(240, _camera.viewBox().max().y() - 240, -.815),
//	                     Vector2(-1, 1), "Boss");
//	_boss = createSprite(&_bossSprite[2],
//	                     Vector3(220, _camera.viewBox().max().y() - 240, -.815),
//	                     Vector2(-1, 1), "Boss");

	Vector3 bhpPos(10, _camera.viewBox().max().y() - 22, 0);
	Vector3 bhpOffset(0, -22, 0);
	for(unsigned i = 0; i < 3; ++i) {
		_bossHealthFull[2-i] = createHealthBar(bhpPos + i * bhpOffset, 1);
	}

	Vector3 closestPos(_camera.viewBox().max().x() - 30,
	                   _camera.viewBox().max().y() - 240, -.8);
	Vector3 offset(-50, 14, -.01);

//	Vector3 namePos(250, 94, -0.01);
	Vector3 namePos(250, 22, -0.01);
//	Vector3 nameOffset(0, -24, 0);
	Vector3 nameOffset(0, 24, 0);
	Vector3 statusPos = namePos + Vector3(110, -4, 0);
	Vector3 statusOffset(16, 0, 0);
	Vector3 hpPos = namePos + Vector3(160, -8, 0);

	_maxPcHp = 0;
	for(PC& pc: _fight->party) {
		_maxPcHp = std::max(_maxPcHp, pc.hp);
	}
	for(unsigned pc = 0; pc < 4; ++pc) {
		_pc[pc] = createSprite(&_pcSprite[pc], closestPos + pc * offset,
		                       Vector2(-1, 1), _fight->party[pc].name.c_str());
		createText(_fight->party[pc].name, namePos + pc * nameOffset);
		_pcMenu->setLabel(3 - pc, _fight->party[pc].name);

		_pcStatus[pc*NB_REAL_STATUS + 0] =
		        createSprite(&_statusSprite, statusPos + pc * nameOffset + 0 * statusOffset);
		_pcStatus[pc*NB_REAL_STATUS + 0].sprite()->setIndex(0);
		_pcStatus[pc*NB_REAL_STATUS + 0].sprite()->setColor(Vector4(1, 1, 1, 0));

		_pcStatus[pc*NB_REAL_STATUS + 1] =
		        createSprite(&_statusSprite, statusPos + pc * nameOffset + 1 * statusOffset);
		_pcStatus[pc*NB_REAL_STATUS + 1].sprite()->setIndex(1);
		_pcStatus[pc*NB_REAL_STATUS + 1].sprite()->setColor(Vector4(1, 1, 1, 0));

		_pcStatus[pc*NB_REAL_STATUS + 2] =
		        createSprite(&_statusSprite, statusPos + pc * nameOffset + 2 * statusOffset);
		_pcStatus[pc*NB_REAL_STATUS + 2].sprite()->setIndex(2);
		_pcStatus[pc*NB_REAL_STATUS + 2].sprite()->setColor(Vector4(1, 1, 1, 0));

		_pcHealthFull[pc] = createHealthBar(hpPos + pc * nameOffset,
		                                    float(_fight->party[pc].hp) / _maxPcHp);
	}
	_pcMenu->layout();

	_game->audio()->playMusic(_music1);

	updateMenu();
}


void MainState::play () {
	if(_state == PLAYING && _messages.empty()) {
		log().warning("Turn");
		if(_fight->game_over()) {
			_state = GAME_OVER;
		} else if(_fight->tick_fight()) {
			_state = BOSS_TURN;
			updateMenu();
			openMenu(_mainMenu.get());
		}
		updateHealthBars();
	}
}


void MainState::updateHealthBars() {
	for(unsigned pc = 0; pc < 4; ++pc) {
		_pcHealthFull[pc].sprite()->setView(
		            Box2(Vector2(0, 0), Vector2(float(_fight->party[pc].hp) / _maxPcHp, 1)));
	}

	float hpRate = _fight->boss_hp_rate();
	if(hpRate > 1) hpRate = 0;
	_bossHealthFull[_fight->tier].sprite()->setView(
		            Box2(Vector2(0, 0), Vector2(hpRate, 1)));
	for(unsigned i = 0; i < _fight->tier; ++i) {
		EntityRef e = _bossHealthFull[i].parent().firstChild();
		while(e.isValid()) {
			e.sprite()->setColor(Vector4(1, 1, 1, 0));
			e = e.nextSibling();
		}
	}
}


void MainState::updateTick() {
}


void MainState::updateFrame() {
	play();

	_inputs.sync();

	if(_debugInput->justPressed()) {
		_state = GAME_OVER;
		_menuStack.clear();
	}

	if(!_messages.empty()) {
		if(_menuInputs.ok->justPressed()
		|| _menuInputs.cancel->justPressed()) {
			nextMessage();
		}
	} else if(!_menuStack.empty()) {
		_menuStack.back()->update();
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	_game->renderer()->mainBatch().clearBuffers();

	_entities.updateWorldTransform();
	_sprites.render(_loop.frameInterp(), _camera);
	_texts.render(  _loop.frameInterp(), _game->renderer());
	_anims.update(_loop.tickDuration());

	_statusFrame->render(_game->renderer());

	if(_messages.empty()) {
		for(Menu* menu: _menuStack) {
			menu->render(_game->renderer());
		}
	} else {
		_messageFrame->render(_game->renderer());

		_font->render(_game->renderer(),
		              Vector3(_messageOutMargin + _messageMargin,
		                      _messageTextHeight, 0.95),
		              Vector4(1, 1, 1, 1),
		              _messages.front());
	}

	_game->renderer()->spriteShader()->use();
	_game->renderer()->spriteShader()->setTextureUnit(0);
	_game->renderer()->spriteShader()->setViewMatrix(_camera.transform());
	_game->renderer()->mainBatch().render();

	_game->window()->swapBuffers();

	uint64 now = _game->sys()->getTimeNs();
	++_fpsCount;
	if(_fpsCount == 60) {
		log().info("Fps: ", _fpsCount * 1000000000. / (now - _fpsTime));
		_fpsTime  = now;
		_fpsCount = 0;
	}

	LAIR_LOG_OPENGL_ERRORS_TO(log());
}


void MainState::showMessage(const std::string& message) {
	_messages.push_back(_font->layoutText(
	                        message, 640 - (_messageMargin + _messageOutMargin) * 2));
	if(_messages.size() == 1) {
		layoutMessage();
	}
}


void MainState::nextMessage() {
	lairAssert(!_messages.empty());
	_messages.pop_front();
	if(!_messages.empty()) {
		layoutMessage();
	}
}


void MainState::layoutMessage() {
	unsigned nLines = std::count(_messages.front().begin(), _messages.front().end(), '\n') + 1;
	_messageFrame->size.y() = _messageMargin * 2 + nLines * _font->height();
	_messageFrame->position.y() = _camera.viewBox().max().y() - 8 - _messageFrame->size.y();
	_messageTextHeight = _camera.viewBox().max().y() - _messageOutMargin
	        - _messageMargin - _font->baselineToTop;
}


void MainState::displayDamages(unsigned target, unsigned damages) {
	lairAssert(target < 5);
	createDamageText(std::to_string(damages), _damagePos[target],
	                 Vector4(1., .2, .0, 1));
}


void MainState::playDeathAnim(unsigned target) {
	lairAssert(target < 5);
	EntityRef e = (target < 4)? _pc[target]: _boss;
	_anims.get(e)->play(new SwapSpriteAnim(ONE_SEC/10, &_tombSprite, 9));
}


void MainState::playRezAnim(unsigned target) {
	lairAssert(target < 4);
	_anims.get(_pc[target])->play(new SwapSpriteAnim(ONE_SEC/10, &_pcSprite[target], 9));
}


void MainState::setStatusVisibility(unsigned pj, RealStatus status, bool visible) {
	unsigned index = pj * NB_REAL_STATUS + status;
	_pcStatus[index].sprite()->setColor(Vector4(1, 1, 1, visible));
}


void MainState::updateMenu() {
	for(unsigned i = 0; i < _fight->rules.party_size; ++i) {
		_pcMenu->setEnabled(3-i, _fight->can_haz(PUNCH, i));
	}

	_mainMenu->setEnabled(MAIN_SWITCH, _fight->can_haz(SWITCH));
	for(unsigned i = 0; i < NB_ELEMS; ++i) {
		_switchMenu->setEnabled(i, _fight->can_haz(SWITCH, i));
	}

	_spellMenu->setEnabled(SPELL_STORM,   _fight->can_haz(STORM));
	_spellMenu->setEnabled(SPELL_STRIKE,  _fight->can_haz(STRIKE));
	_spellMenu->setEnabled(SPELL_CRIPPLE, _fight->can_haz(CRIPPLE));
	_spellMenu->setEnabled(SPELL_DRAIN,   _fight->can_haz(DRAIN));
	_spellMenu->setEnabled(SPELL_VORPAL,  _fight->can_haz(VORPAL));
	_spellMenu->setEnabled(SPELL_MUD,     _fight->can_haz(MUD));
	_spellMenu->setEnabled(SPELL_DISPEL,  _fight->can_haz(DISPEL));
}


void MainState::openMenu(Menu* menu, Menu* parent, unsigned entry) {
	if(parent) {
		menu->_frame.position.head<2>() = parent->_frame.position.head<2>()
		                                + parent->_frame.size;
		menu->_frame.position.y() -=
		        menu->height() + _font->height() * entry;
		menu->_frame.position.y() = std::max(menu->_frame.position.y(), 0.f);
	}
	_menuStack.push_back(menu);
}


Menu::Callback MainState::openMenuFunc(Menu* menu, Menu* parent, unsigned entry) {
	return std::bind(&MainState::openMenu, this, menu, parent, entry);
}


void MainState::closeMenu() {
	_menuStack.pop_back();
}


Menu::Callback MainState::closeMenuFunc() {
	return std::bind(&MainState::closeMenu, this);
}


Logger& MainState::log() {
	return _game->log();
}


void MainState::doAction() {
	lairAssert(!_menuStack.empty());
	int target = 3 - _menuStack.back()->selected();
	switch(_menuStack[0]->selected()) {
	case MAIN_ATTACK: {
		lairAssert(_menuStack.size() == 2);
		_fight->curse(PUNCH, target);
		break;
	}
	case MAIN_SWITCH: {
		lairAssert(_menuStack.size() == 2);
		switch(_menuStack[1]->selected()) {
		case NONE:
			_fight->curse(SWITCH, NONE);
			break;
		case FIRE:
			_fight->curse(SWITCH, FIRE);
			break;
		case ICE:
			_fight->curse(SWITCH, ICE);
			break;
		case SPARK:
			_fight->curse(SWITCH, SPARK);
			break;
		case ACID:
			_fight->curse(SWITCH, ACID);
			break;
		}
		break;
	}
	case MAIN_SPELL: {
		lairAssert(_menuStack.size() >= 2);
		switch(_menuStack[1]->selected()) {
		case SPELL_STORM:
			lairAssert(_menuStack.size() == 2);
			_fight->curse(STORM, -1);
			break;
		case SPELL_STRIKE:
			lairAssert(_menuStack.size() == 3);
			_fight->curse(STRIKE, target);
			break;
		case SPELL_CRIPPLE:
			lairAssert(_menuStack.size() == 3);
			_fight->curse(CRIPPLE, target);
			break;
		case SPELL_DRAIN:
			lairAssert(_menuStack.size() == 3);
			_fight->curse(DRAIN, target);
			break;
		case SPELL_VORPAL:
			lairAssert(_menuStack.size() == 3);
			_fight->curse(VORPAL, target);
			break;
		case SPELL_MUD:
			lairAssert(_menuStack.size() == 2);
			_fight->curse(MUD, -1);
			break;
		case SPELL_DISPEL:
			lairAssert(_menuStack.size() == 3);
			_fight->curse(DISPEL, target);
			break;
		}
		break;
	}
	case 3: { // summon
		break;
	}
	}

	updateHealthBars();

	_state = PLAYING;
	while(!_menuStack.empty()) {
		closeMenu();
	}
}


Menu::Callback MainState::doActionFunc() {
	return std::bind(&MainState::doAction, this);
}
