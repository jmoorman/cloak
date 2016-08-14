#include "stdafx.h"
#include "Animation.h"


Animation::Animation(void)
{
}


Animation::~Animation(void)
{
}

bool Animation::loadAnimation(const std::string& filename) {
    //attempt to open the file
	std::ifstream file(filename);
	if(file.fail()) {
		return false;
	}

	//get file length
	file.seekg(0, std::ios::end);
	int fileLength = (int)file.tellg();
    file.seekg(std::ios::beg);

	std::string token;
	std::string ignore;
	
	while(file >> token, !file.eof()) {
		if(token == "numFrames") {
            file >> mFrameCount;	
		}
        else if (token == "numJoints") {
            file >> mBoneCount;
        }
        else if (token == "frameRate") {
            file >> mFrameRate;
		}
        else if (token == "numAnimatedComponents") {
            file >> mComponentCount;
        }
        else if (token == "hierarchy") {
            file >> ignore; // opening {
            for(int i = 0; i < mBoneCount; i++) {
                BoneInfo boneInfo;
                file >> boneInfo.name >> boneInfo.parentId >> boneInfo.flags >> boneInfo.startIndex;
                size_t n;
                while ((n = boneInfo.name.find('\"') ) != std::string::npos ) boneInfo.name.erase(n,1);
                mBoneInfos.push_back(boneInfo);
               	file.ignore(fileLength, '\n'); //ignore comments
            }
            file >> ignore; // closing }
        }
        else if (token == "bounds") {
            file >> ignore; // opening {
            for(int i = 0; i < mFrameCount; i++) {
                AABoundingBox boundingBox;
                file >> ignore; // opening (
                file >> boundingBox.min.x >> boundingBox.min.y >> boundingBox.min.z;
                file >> ignore >> ignore; // closing ), opening (
                file >> boundingBox.max.x >> boundingBox.max.y >> boundingBox.max.z;
                file >> ignore; // closing )
                mBounds.push_back(boundingBox);
            }
            file >> ignore; // closing }
        }
        else if (token == "baseframe") {
            file >> ignore; // opening {
            for(int i = 0; i < mBoneCount; i++) {
                BaseFrame baseFrame;
                file >> ignore; // opening (
                file >> baseFrame.position.x >> baseFrame.position.y >> baseFrame.position.z;
                file >> ignore >> ignore; // closing ), opening (
                file >> baseFrame.orientation.x >> baseFrame.orientation.y >> baseFrame.orientation.z;
                file >> ignore; // closing )
                mBaseFrames.push_back(baseFrame);
            }
            file >> ignore; // closing }
        }
        else if (token == "frame") {
            FrameData frame;
            file >> frame.frameId;
            file >> ignore; // Read in the '{' character
            for ( int i = 0; i < mComponentCount; ++i )
            {
                float data;
                file >> data;
                frame.frameData.push_back(data);
            }
            mFrames.push_back(frame);

            // Build a skeleton for this frame
            BuildFrameSkeleton(mSkeletons, mBoneInfos, mBaseFrames, frame);
            file >> ignore; // Read in the '}' character       
        }
	}
    mAnimatedSkeleton.bones.assign(mBoneCount, SkeletonBone());
    mFrameDuration = 1.0f / (float)mFrameRate;
    mAnimationDuration = mFrameDuration * mFrameCount;
    mAnimationTime = 0.0f;

    assert(mBoneInfos.size() == mBoneCount);
    assert(mBaseFrames.size() == mBoneCount);
    assert(mFrames.size() == mFrameCount);
    assert(mBounds.size() == mFrameCount);
    assert(mSkeletons.size() == mFrameCount);
    return true;
}

void Animation::saveAnimation() {
	std::cout << "MD5Version 10" << std::endl;
	std::cout << "commandline \"\"" << std::endl << std::endl;
	
    std::cout << "numFrames " << mFrames.size() << std::endl;
    std::cout << "numJoints " << mBoneInfos.size() << std::endl;
    std::cout << "frameRate " << mFrameRate << std::endl;
    std::cout << "numAnimatedComponents " << mComponentCount << std::endl;
    std::cout << std::endl;
	
    std::cout << "hierarchy {" << std::endl;
    for(int i = 0; i < mBoneInfos.size(); i++) {
        const BoneInfo &boneInfo = mBoneInfos[i];
        std::cout << "\t\"" << boneInfo.name << "\"\t" << boneInfo.parentId << " " << boneInfo.flags << " " << boneInfo.startIndex << std::endl;
    }
    std::cout << "}" << std::endl;

    std::cout << "bounds {" << std::endl;
    for(int i = 0; i < mFrameCount; i++) {
        const AABoundingBox &bound = mBounds[i];
        std::cout << std::fixed << std::setprecision(6)
                << "\t( " << bound.min.x << " " << bound.min.y << " " << bound.min.z << " )"
                << " ( " << bound.max.x << " " << bound.max.y << " " << bound.max.z << " )" << std::endl;
    }
    std::cout << "}" << std::endl;

    std::cout << "baseframe {" << std::endl;
    for(int i = 0; i < mBaseFrames.size(); i++)
    {
        const BaseFrame &baseFrame = mBaseFrames[i];
        std::cout << std::fixed << std::setprecision(6)
                << "\t( " << baseFrame.position.x << " " << baseFrame.position.y << " " << baseFrame.position.z << " )"
                << " ( " << baseFrame.orientation.x << " " << baseFrame.orientation.y << " " << baseFrame.orientation.z << " )" << std::endl;
    }
    std::cout << "}" << std::endl;
    
    for(int i = 0; i < mFrames.size(); i++)
    {
        std::cout << std::endl;
        const FrameData &frame = mFrames[i];
        std::cout << "frame " << i << " {";
        for(int j = 0; j < frame.frameData.size(); j++) {

            if(j % 6 == 0) {
                std::cout << std::endl << "\t";
            }
            else {
                std::cout << " ";
            }
            std::cout << frame.frameData[j];
        }
        std::cout << std::endl << "}" << std::endl;
    }
}

void Animation::update(unsigned int elapsedTimeInMillis) {
    if(mFrameCount < 1) return;

    mAnimationTime += (elapsedTimeInMillis / 1000.f);
    while(mAnimationTime > mAnimationDuration) mAnimationTime -= mAnimationDuration;
    while(mAnimationTime < 0.0f) mAnimationTime += mAnimationDuration;

    //Figure out which frame we're on
    float frameNum = mAnimationTime * (float)mFrameRate;
    int frame0 = (int)floorf(frameNum);
    int frame1 = (int)ceilf(frameNum);
    frame0 = frame0 % mFrameCount;
    frame1 = frame1 % mFrameCount;

    float interpolate = fmodf(mAnimationTime, mFrameDuration) / mFrameDuration;
    InterpolateSkeletons(mAnimatedSkeleton, mSkeletons[frame0], mSkeletons[frame1], interpolate);
}

const Animation::FrameSkeleton& Animation::getSkeleton() const {
	return mAnimatedSkeleton;
}

int Animation::getBoneCount() const {
	return mBoneCount;
}

const Animation::BoneInfo& Animation::getBoneInfo(unsigned int index) const {
	return mBoneInfos[index];
}

void Animation::BuildFrameSkeleton(FrameSkeletonList& skeletons, const BoneInfoList& boneInfos, const BaseFrameList& baseFrames, const FrameData& frameData) {
    FrameSkeleton skeleton;
    for(int i = 0; i < boneInfos.size(); i++)
    {
        unsigned int j = 0;
        const BoneInfo &boneInfo = boneInfos[i];
        // Start with base frame position/orientation
        SkeletonBone animatedBone = baseFrames[i];
        animatedBone.parentId = boneInfo.parentId;
        
        if(boneInfo.flags & 1)
        {
            animatedBone.position.x = frameData.frameData[boneInfo.startIndex + j++];
        }
        if(boneInfo.flags & 2)
        {
            animatedBone.position.y = frameData.frameData[boneInfo.startIndex + j++];
        }
        if(boneInfo.flags & 4)
        {
            animatedBone.position.z = frameData.frameData[boneInfo.startIndex + j++];
        }
        if(boneInfo.flags & 8)
        {
            animatedBone.orientation.x = frameData.frameData[boneInfo.startIndex + j++];
        }
        if(boneInfo.flags & 16)
        {
            animatedBone.orientation.y = frameData.frameData[boneInfo.startIndex + j++];
        }
        if(boneInfo.flags & 32)
        {
            animatedBone.orientation.z = frameData.frameData[boneInfo.startIndex + j++];
        }
        float t = 1.0f - 
            (animatedBone.orientation.x * animatedBone.orientation.x) - 
            (animatedBone.orientation.y * animatedBone.orientation.y) - 
            (animatedBone.orientation.z * animatedBone.orientation.z);
        if(t < 0.0f)
        {
            animatedBone.orientation.w = 0.0f;
        }
        else
        {
            animatedBone.orientation.w = -sqrtf(t);
        }

        if(animatedBone.parentId >=0)
        {
            SkeletonBone &parentBone = skeleton.bones[animatedBone.parentId];
            glm::vec3 rotPos = parentBone.orientation * animatedBone.position;
            animatedBone.position = parentBone.position + rotPos;
            animatedBone.orientation = parentBone.orientation * animatedBone.orientation;
            animatedBone.orientation = glm::normalize(animatedBone.orientation);
        }
        skeleton.bones.push_back(animatedBone);
    }
    skeletons.push_back(skeleton);
}

void Animation::InterpolateSkeletons(FrameSkeleton& result, const FrameSkeleton& skeleton0, const FrameSkeleton& skeleton1, float blendWeight) {
    for(int i = 0; i < mBoneCount; i++) {
        SkeletonBone& resultBone = result.bones[i];
        const SkeletonBone &bone0 = skeleton0.bones[i];
        const SkeletonBone &bone1 = skeleton1.bones[i];

        resultBone.parentId = bone0.parentId;
        resultBone.position = glm::lerp(bone0.position, bone1.position, blendWeight);
        resultBone.orientation = glm::mix(bone0.orientation, bone1.orientation, blendWeight);
    }
}
