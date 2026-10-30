#include "Entity.h"

Entity::Entity() :
    mPosition{ 0.0f, 0.0f }, mMovement{ 0.0f, 0.0f },
    mVelocity{ 0.0f, 0.0f }, mAcceleration{ 0.0f, 0.0f },
    mScale{ DEFAULT_SIZE, DEFAULT_SIZE },
    mColliderDimensions{ DEFAULT_SIZE, DEFAULT_SIZE },
    mTexture{}, mTextureType{ SINGLE }, mSpriteSheetDimensions{},
    mAnimationAtlas{}, mAnimationIndices{}, mDirection{ RIGHT },
    mFrameSpeed{ 0 }, mSpeed{ DEFAULT_SPEED }, mAngle{ 0.0f } {}

Entity::Entity(Vector2 position, Vector2 scale, const char *textureFilepath) :
    mPosition{ position }, mMovement{ 0.0f, 0.0f },
    mVelocity{ 0.0f, 0.0f }, mAcceleration{ 0.0f, 0.0f },
    mScale{ scale }, mColliderDimensions{ scale },
    mTexture{ LoadTexture(textureFilepath) }, mTextureType{ SINGLE },
    mSpriteSheetDimensions{}, mAnimationAtlas{}, mAnimationIndices{},
    mDirection{ RIGHT }, mFrameSpeed{ 0 }, mSpeed{ DEFAULT_SPEED },
    mAngle{ 0.0f } {}

Entity::Entity(
    Vector2                               position,
    Vector2                               scale,
    const char                           *textureFilepath,
    TextureType                           textureType,
    Vector2                               spriteSheetDimensions,
    std::map<Direction, std::vector<int>> animationAtlas
) :
    mPosition{ position }, mMovement{ 0.0f, 0.0f },
    mVelocity{ 0.0f, 0.0f }, mAcceleration{ 0.0f, 0.0f },
    mScale{ scale }, mColliderDimensions{ scale },
    mTexture{ LoadTexture(textureFilepath) }, mTextureType{ textureType },
    mSpriteSheetDimensions{ spriteSheetDimensions },
    mAnimationAtlas{ animationAtlas },
    mAnimationIndices{ animationAtlas.at(RIGHT) }, mDirection{ RIGHT },
    mFrameSpeed{ DEFAULT_FRAME_SPEED }, mSpeed{ DEFAULT_SPEED },
    mAngle{ 0.0f } {}

Entity::~Entity() { UnloadTexture(mTexture); }

/**
 * Updates the current frame index of an entity's animation based on the
 * elapsed time and frame speed.
 *
 * @param deltaTime represents the time elapsed since the last frame update.
 */
void Entity::animate(float deltaTime)
{
    mAnimationIndices = mAnimationAtlas.at(mDirection);

    mAnimationTime += deltaTime;
    float secondsPerFrame = 1.0f / mFrameSpeed;

    if (mAnimationTime >= secondsPerFrame)
    {
        mAnimationTime = 0.0f;

        mCurrentFrameIndex++;
        mCurrentFrameIndex %= mAnimationIndices.size();
    }
}

void Entity::update(float deltaTime)
{
    mIsCollidingTop = false;
    mIsCollidingBottom = false;
    mIsCollidingLeft = false;
    mIsCollidingRight = false;

    mVelocity.x += mAcceleration.x * deltaTime;
    mVelocity.y += mAcceleration.y * deltaTime;

    // Gradually slow horizontal drift when no horizontal thrust is applied.
    if (mAcceleration.x == 0.0f)
    {
        float drag = 65.0f * deltaTime;
        if (mVelocity.x > drag) mVelocity.x -= drag;
        else if (mVelocity.x < -drag) mVelocity.x += drag;
        else mVelocity.x = 0.0f;
    }

    mPosition.x += mVelocity.x * deltaTime;
    mPosition.y += mVelocity.y * deltaTime;

    if (mTextureType == ATLAS) animate(deltaTime);
}

bool Entity::isColliding(Entity *other) const
{
    float xDistance = fabs(mPosition.x - other->getPosition().x) -
        (mColliderDimensions.x + other->getColliderDimensions().x) / 2.0f;
    float yDistance = fabs(mPosition.y - other->getPosition().y) -
        (mColliderDimensions.y + other->getColliderDimensions().y) / 2.0f;
    return xDistance < 0.0f && yDistance < 0.0f;
}

void Entity::update(float deltaTime, Entity *collidableEntities, int collisionCheckCount)
{
    Vector2 oldPosition = mPosition;
    update(deltaTime);
    for (int i = 0; i < collisionCheckCount; i++)
    {
        Entity *other = &collidableEntities[i];
        if (!isColliding(other)) continue;

        Vector2 pos = other->getPosition();
        Vector2 size = other->getColliderDimensions();
        float halfWidth = (mColliderDimensions.x + size.x) / 2.0f;
        float halfHeight = (mColliderDimensions.y + size.y) / 2.0f;

        // Determine the collision face using the previous position.
        if (oldPosition.y + mColliderDimensions.y / 2.0f <= pos.y - size.y / 2.0f && mVelocity.y >= 0.0f)
        {
            mIsCollidingBottom = true;
            mPosition.y = pos.y - halfHeight;
            mVelocity.y = 0.0f;
        }
        else if (oldPosition.y - mColliderDimensions.y / 2.0f >= pos.y + size.y / 2.0f && mVelocity.y <= 0.0f)
        {
            mIsCollidingTop = true;
            mPosition.y = pos.y + halfHeight;
            mVelocity.y = 0.0f;
        }
        else if (oldPosition.x <= pos.x)
        {
            mIsCollidingRight = true;
            mPosition.x = pos.x - halfWidth;
            mVelocity.x = 0.0f;
        }
        else
        {
            mIsCollidingLeft = true;
            mPosition.x = pos.x + halfWidth;
            mVelocity.x = 0.0f;
        }
    }
}

void Entity::render()
{
    Rectangle textureArea;

    switch (mTextureType)
    {
    case SINGLE:
        // Whole texture (UV coordinates)
        textureArea = {
            // top-left corner
            0.0f, 0.0f,

            // bottom-right corner (of texture)
            static_cast<float>(mTexture.width),
            static_cast<float>(mTexture.height)
        };
        break;
    case ATLAS:
        textureArea = getUVRectangle(
            &mTexture,
            mAnimationIndices[mCurrentFrameIndex],
            mSpriteSheetDimensions.x,
            mSpriteSheetDimensions.y
        );
        break;
    default:
        break;
    }

    // Destination rectangle – centred on mPosition
    Rectangle destinationArea = {
        mPosition.x,
        mPosition.y,
        static_cast<float>(mScale.x),
        static_cast<float>(mScale.y)
    };

    // Origin inside the source texture (centre of the texture)
    Vector2 originOffset = {
        static_cast<float>(mScale.x) / 2.0f,
        static_cast<float>(mScale.y) / 2.0f
    };

    // Render the texture on screen
    DrawTexturePro(
        mTexture,
        textureArea, destinationArea, originOffset,
        mAngle, WHITE
    );
}
