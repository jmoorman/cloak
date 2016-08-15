#pragma once

#include "stdafx.h"

class Animation
{
public:
	struct BoneInfo
	{
		std::string name;
		int parentId;
		int flags;
		int startIndex;
	};
	typedef std::vector<BoneInfo> BoneInfoList;

	struct AABoundingBox
	{
		glm::vec3 min;
		glm::vec3 max;
	};
	typedef std::vector<AABoundingBox> BoundsList;

	struct BaseFrame
	{
		glm::vec3 position;
		glm::quat orientation;
	};
	typedef std::vector<BaseFrame> BaseFrameList;

	struct FrameData
	{
		int frameId;
		std::vector<float> frameData;
	};
	typedef std::vector<FrameData> FrameDataList;

	struct SkeletonBone
	{
		SkeletonBone() : parentId(-1), position(0) {}
		SkeletonBone(const BaseFrame& copy)
			: position(copy.position), orientation(copy.orientation) {}
		
		int parentId;
		glm::vec3 position;
		glm::quat orientation;
	};
	typedef std::vector<SkeletonBone> SkeletonBoneList;

	struct FrameSkeleton
	{
		SkeletonBoneList bones;
	};
	typedef std::vector<FrameSkeleton> FrameSkeletonList;

	Animation(void);
	~Animation(void);
	bool loadAnimation(const std::string& filename);
    void saveAnimation();
	void update(unsigned int elapsedTimeInMillis);
	const FrameSkeleton& getSkeleton() const;
	int getBoneCount() const;
	const BoneInfo& getBoneInfo(unsigned int index) const;

private:
	BoneInfoList mBoneInfos;
	BoundsList mBounds;
	BaseFrameList mBaseFrames;
	FrameDataList mFrames;
	FrameSkeletonList mSkeletons;

	FrameSkeleton mAnimatedSkeleton;

	int mFrameCount;
	int mBoneCount;
	int mFrameRate;
	int mComponentCount;

	float mAnimationDuration;
	float mFrameDuration;
	float mAnimationTime;

	void BuildFrameSkeleton(FrameSkeletonList& skeletons, const BoneInfoList& boneInfos, const BaseFrameList& baseFrames, const FrameData& frameData);
	void InterpolateSkeletons(FrameSkeleton& result, const FrameSkeleton& skeleton0, const FrameSkeleton& skeleton1, float blendWeight);
};

