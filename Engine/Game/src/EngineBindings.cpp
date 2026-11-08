/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** EngineBindings
*/

#include "AngularVelocity2D.hpp"
#include "AngularVelocity3D.hpp"
#include "Camera3D.hpp"
#include "CameraAngle.hpp"
#include "CameraControl.hpp"
#include "Clock.hpp"
#include "ComponentBinding.hpp"
#include "Draw3D.hpp"
#include "Font.hpp"
#include "Friction2D.hpp"
#include "Friction3D.hpp"
#include "GameManagerBinding.hpp"
#include "GravityField2D.hpp"
#include "GravityField3D.hpp"
#include "Keybind.hpp"
#include "KeyboardManager.hpp"
#include "Listener.hpp"
#include "Model3D.hpp"
#include "ModelAnimation3D.hpp"
#include "Origin2D.hpp"
#include "Position2D.hpp"
#include "Position3D.hpp"
#include "Size2D.hpp"
#include "Sound.hpp"
#include "Sprite2D.hpp"
#include "Sprite3D.hpp"
#include "SpriteSheet2D.hpp"
#include "SpriteSheet3D.hpp"
#include "SystemBinding.hpp"
#include "Text2D.hpp"
#include "Texture2D.hpp"
#include "Velocity2D.hpp"
#include "Velocity3D.hpp"

namespace RType::Game {

    const std::unordered_map<std::string, SystemBinding::Factory>
        SystemBinding::engineSystems = {
            // engine-system
            {"Clock", makeSystem(Clock::systemClock)},
            // engine-graphical2D
            {"KeyboardManagerClear",
                makeSystem(KeyboardManager::systemKeyboardManagerClear)},
            {"UpdateKeybind", makeSystem(Keybind::systemUpdateKeybind)},
            {"DrawText2D", makeSystem(Text2D::systemDrawText2D)},
            {"DrawSprite2D", makeSystem(Sprite2D::systemDrawSprite2D)},
            {"DrawSpriteSheet2D",
                makeSystem(SpriteSheet2D::systemDrawSpriteSheet2D)},
            {"AnimateSpriteSheet2D",
                makeSystem(SpriteSheet2D::systemAnimateSpriteSheet2D)},
            {"MasterAnimateSpriteSheet2D",
                makeSystem(SpriteSheet2D::systemMasterAnimateSpriteSheet2D)},
            // engine-graphical3D
            {"Draw3D", makeSystem(Draw3D::systemDraw3D)},
            {"DrawSprite3D", makeSystem(Sprite3D::systemDrawSprite3D)},
            {"UpdateCamera", makeSystem(Camera3D::systemUpdateCamera)},
            {"AnimateSpriteSheet3D",
                makeSystem(SpriteSheet3D::systemAnimateSpriteSheet3D)},
            {"MasterAnimateSpriteSheet3D",
                makeSystem(SpriteSheet3D::systemMasterAnimateSpriteSheet3D)},
            {"DrawSpriteSheet3D",
                makeSystem(SpriteSheet3D::systemDrawSpriteSheet3D)},
            // engine-audio
            {"Sound2D", makeSystem(Sound::systemSound2D)},
            // engine-physics
            {"Velocity2D", makeSystem(Velocity2D::systemVelocity2D)},
            {"MasterVelocity2D",
                makeSystem(Velocity2D::systemMasterVelocity2D)},
            {"Friction2D", makeSystem(Friction2D::systemFriction2D)},
            {"MasterFriction2D",
                makeSystem(Friction2D::systemMasterFriction2D)},
            {"GravityField2D",
                makeSystem(GravityField2D::systemGravityField2D)},
            {"MasterGravityField2D",
                makeSystem(GravityField2D::systemMasterGravityField2D)},
            {"Velocity3D", makeSystem(Velocity3D::systemVelocity3D)},
            {"MasterVelocity3D",
                makeSystem(Velocity3D::systemMasterVelocity3D)},
            {"Friction3D", makeSystem(Friction3D::systemFriction3D)},
            {"MasterFriction3D",
                makeSystem(Friction3D::systemMasterFriction3D)},
            {"GravityField3D",
                makeSystem(GravityField3D::systemGravityField3D)},
            {"MasterGravityField3D",
                makeSystem(GravityField3D::systemMasterGravityField3D)},
            {"AngularVelocity2D",
                makeSystem(AngularVelocity2D::systemAngularVelocity2D)},
            {"MasterAngularVelocity2D",
                makeSystem(AngularVelocity2D::systemMasterAngularVelocity2D)},
            {"AngularVelocity3D",
                makeSystem(AngularVelocity3D::systemAngularVelocity3D)},
            {"MasterAngularVelocity3D",
                makeSystem(AngularVelocity3D::systemMasterAngularVelocity3D)},
            {"CameraAngleInput",
                makeSystem(CameraAngle::systemCameraAngleInput)},
            {"CameraAngle", makeSystem(CameraAngle::systemCameraAngle)},
            {"CameraMouseControl",
                makeSystem(CameraMouseControl::systemCameraMouseControl)},
    };

    template<>
    Sound ComponentBinding::construct<Sound>(
        GameManager &gm, const std::string &path)
    {
        return {GameManagerBinding::findModuleComponent<Listener>(
                    gm, "Sound", "Audio"),
            path};
    }

    void ComponentBinding::bindEngineComponents(sol::state &lua)
    {
        // engine-system
        bindComponent<Position2D>(
            lua, "Position2D", "x", &Position2D::x, "y", &Position2D::y);
        bindComponent<Position3D>(lua,
            "Position3D",
            "x",
            &Position3D::x,
            "y",
            &Position3D::y,
            "z",
            &Position3D::z);
        bindComponent<Origin2D>(
            lua, "Origin2D", "x", &Origin2D::x, "y", &Origin2D::y);
        bindComponent<Size2D>(
            lua, "Size2D", "height", &Size2D::height, "width", &Size2D::width);
        bindComponent<Clock>(lua,
            "Clock",
            "speed",
            &Clock::speed,
            "elapsed",
            [](const Clock &clock) { return clock.getElapsed().count(); });

        // engine-graphical2D
        bindComponent<Texture2D>(lua, "Texture2D");
        bindComponent<Font>(lua, "Font");
        bindComponent<Sprite2D>(lua,
            "Sprite2D",
            "scale",
            &Sprite2D::scale,
            "rotation",
            &Sprite2D::rotation,
            "flip",
            &Sprite2D::flip);
        bindComponent<SpriteSheet2D>(lua,
            "SpriteSheet2D",
            "rows",
            nested(&SpriteSheet2D::recNb, &Size2D::height),
            "columns",
            nested(&SpriteSheet2D::recNb, &Size2D::width),
            "currentTile",
            &SpriteSheet2D::currentTile,
            "frameDuration",
            milliseconds(&SpriteSheet2D::frameDuration),
            "animation",
            &SpriteSheet2D::animation,
            "scale",
            &SpriteSheet2D::scale,
            "rotation",
            &SpriteSheet2D::rotation,
            "flip",
            &SpriteSheet2D::flip);
        bindComponent<Text2D>(lua,
            "Text2D",
            "str",
            &Text2D::str,
            "fontSize",
            &Text2D::fontSize,
            "rotation",
            &Text2D::rotation,
            "spacing",
            &Text2D::spacing);

        // engine-graphical3D
        bindComponent<Model3D>(lua, "Model3D");
        bindComponent<ModelAnimation3D>(lua, "ModelAnimation3D");
        bindComponent<Sprite3D>(lua, "Sprite3D", "scale", &Sprite3D::scale);
        bindComponent<SpriteSheet3D>(lua,
            "SpriteSheet3D",
            "index",
            &SpriteSheet3D::index,
            "frameDuration",
            milliseconds(&SpriteSheet3D::frameDuration),
            "animation",
            &SpriteSheet3D::animation,
            "scale",
            &SpriteSheet3D::scale);

        // engine-audio
        lua.new_enum<SoundInfo::State>("SoundState",
            {
                {"PLAY", SoundInfo::State::PLAY},
                {"PLAYING", SoundInfo::State::PLAYING},
                {"PAUSED", SoundInfo::State::PAUSED},
                {"START", SoundInfo::State::START},
                {"STOP", SoundInfo::State::STOP},
            });
        lua.new_enum<SoundInfo::Attenuation>("Attenuation",
            {
                {"NONE", SoundInfo::Attenuation::NONE},
                {"INVERSE", SoundInfo::Attenuation::INVERSE},
                {"LINEAR", SoundInfo::Attenuation::LINEAR},
                {"EXPONENTIAL", SoundInfo::Attenuation::EXPONENTIAL},
            });
        bindComponent<Sound>(lua,
            "Sound",
            "state",
            sol::property([](const Sound &s) { return s.state; },
                [](Sound &s, SoundInfo::State state) { s.state = state; }),
            "loop",
            sol::property([](const Sound &s) { return s.loop; },
                [](Sound &s, bool loop) { s.loop = loop; }),
            "stereo",
            sol::property([](const Sound &s) { return s.stereo; },
                [](Sound &s, bool stereo) { s.stereo = stereo; }),
            "attenuation",
            sol::property([](const Sound &s) { return s.attenuation; },
                [](Sound &s, SoundInfo::Attenuation attenuation) {
                    s.attenuation = attenuation;
                }),
            "minDistance",
            sol::property([](const Sound &s) { return s.minDistance; },
                [](Sound &s, float distance) { s.minDistance = distance; }),
            "maxDistance",
            sol::property([](const Sound &s) { return s.maxDistance; },
                [](Sound &s, float distance) { s.maxDistance = distance; }),
            "setVolume",
            &Sound::setVolume,
            "setPitch",
            &Sound::setPitch);

        // engine-physics
        bindComponent<Rotation2D>(
            lua, "Rotation2D", "angle", &Rotation2D::angle);
        bindComponent<Rotation3D>(lua,
            "Rotation3D",
            "x",
            &Rotation3D::x,
            "y",
            &Rotation3D::y,
            "z",
            &Rotation3D::z,
            "w",
            &Rotation3D::w);
        bindComponent<Velocity2D>(
            lua, "Velocity2D", "x", &Velocity2D::x, "y", &Velocity2D::y);
        bindComponent<Friction2D>(lua, "Friction2D", "f", &Friction2D::f);
        bindComponent<GravityField2D>(lua,
            "GravityField2D",
            "x",
            &GravityField2D::x,
            "y",
            &GravityField2D::y,
            "g",
            &GravityField2D::g);
        bindComponent<Velocity3D>(lua,
            "Velocity3D",
            "x",
            &Velocity3D::x,
            "y",
            &Velocity3D::y,
            "z",
            &Velocity3D::z);
        bindComponent<Friction3D>(lua, "Friction3D", "f", &Friction3D::f);
        bindComponent<GravityField3D>(lua,
            "GravityField3D",
            "x",
            &GravityField3D::x,
            "y",
            &GravityField3D::y,
            "z",
            &GravityField3D::z,
            "g",
            &GravityField3D::g);
        bindComponent<AngularVelocity2D>(
            lua, "AngularVelocity2D", "w", &AngularVelocity2D::w);
        bindComponent<AngularVelocity3D>(lua,
            "AngularVelocity3D",
            "x",
            &AngularVelocity3D::x,
            "y",
            &AngularVelocity3D::y,
            "z",
            &AngularVelocity3D::z);
        lua.new_enum<CameraProjection>("CameraProjection",
            {
                {"PERSPECTIVE", CAMERA_PERSPECTIVE},
                {"ORTHOGRAPHIC", CAMERA_ORTHOGRAPHIC},
            });
        lua.new_enum<CameraMode>("CameraMode",
            {
                {"CUSTOM", CAMERA_CUSTOM},
                {"FREE", CAMERA_FREE},
                {"ORBITAL", CAMERA_ORBITAL},
                {"FIRST_PERSON", CAMERA_FIRST_PERSON},
                {"THIRD_PERSON", CAMERA_THIRD_PERSON},
            });
        bindComponent<Camera3D>(lua,
            "Camera3D",
            "targetX",
            nested<Camera3D, ::Vector3, float>(
                &Camera3D::target, &::Vector3::x),
            "targetY",
            nested<Camera3D, ::Vector3, float>(
                &Camera3D::target, &::Vector3::y),
            "targetZ",
            nested<Camera3D, ::Vector3, float>(
                &Camera3D::target, &::Vector3::z),
            "upX",
            nested<Camera3D, ::Vector3, float>(&Camera3D::up, &::Vector3::x),
            "upY",
            nested<Camera3D, ::Vector3, float>(&Camera3D::up, &::Vector3::y),
            "upZ",
            nested<Camera3D, ::Vector3, float>(&Camera3D::up, &::Vector3::z),
            "fovy",
            sol::property([](const Camera3D &c) { return c.fovy; },
                [](Camera3D &c, float fovy) { c.fovy = fovy; }),
            "projection",
            sol::property([](const Camera3D &c) { return c.projection; },
                [](Camera3D &c, int projection) { c.projection = projection; }),
            "updateMode",
            &Camera3D::updateMode);
        bindComponent<CameraAngleInput>(lua,
            "CameraAngleInput",
            "yaw",
            &CameraAngleInput::yaw,
            "pitch",
            &CameraAngleInput::pitch);
        bindComponent<CameraAngle>(lua,
            "CameraAngle",
            "yaw",
            &CameraAngle::yaw,
            "pitch",
            &CameraAngle::pitch,
            "minPitch",
            &CameraAngle::minPitch,
            "maxPitch",
            &CameraAngle::maxPitch);
        bindComponent<CameraMouseControl>(lua,
            "CameraMouseControl",
            "sensitivity",
            &CameraMouseControl::sensitivity,
            "invertY",
            &CameraMouseControl::invertY);
    }

} // namespace RType::Game
