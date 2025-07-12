#include "epch.h"
#include "JobSystem.h"

namespace Epoch::Core
{
	void JobSystem::Init()
	{
		SYSTEM_INFO sysinfo;
		GetSystemInfo(&sysinfo);
		unsigned cpuCount = sysinfo.dwNumberOfProcessors;
		myWorkerCount = std::max(1u, static_cast<unsigned>(cpuCount * 0.8f));

		myRunning = true;

		for (unsigned i = 0; i < myWorkerCount; ++i)
		{
			myWorkers.emplace_back(
				[this]
				{
					WorkerLoop();
				});
		}
	}

	void JobSystem::ShutDown()
	{
		{
			std::unique_lock<std::mutex> lock(myMutex);
			myRunning = false;
		}
		myCondition.notify_all();
		for (std::thread& worker : myWorkers)
		{
			worker.join();
		}
	}

	bool JobSystem::AllTasksDone()
	{
		std::lock_guard<std::mutex> lock(myMutex);
		return
			myActiveTasks == 0 &&
			myHighPriorityJobs.empty() &&
			myNormalPriorityJobs.empty() &&
			myLowPriorityJobs.empty();
	}

	void JobSystem::WaitUntilDone()
	{
		std::unique_lock<std::mutex> lock(myMutex);
		myDoneCondition.wait(lock, [this]() { return AllTasksDone(); });
	}

	void JobSystem::WorkerLoop()
	{
		EPOCH_PROFILE_FUNC("WorkerLoop");
		while (true)
		{
			Job job;
			{
				std::unique_lock<std::mutex> lock(myMutex);
				myCondition.wait(lock, [this]
								 {
									 return !myHighPriorityJobs.empty() || !myNormalPriorityJobs.empty() || !myLowPriorityJobs.empty() || !myRunning;
								 });

				if (!myRunning) return;

				if (!PopReadyJob(job)) continue;
				++myActiveTasks;
			}

			{
				EPOCH_PROFILE_SCOPE(job.Name.c_str());
				job.Task();
			}

			{
				std::lock_guard<std::mutex> lock(myMutex);
				--myActiveTasks;
			}
			{
				if (AllTasksDone())
				{
					myDoneCondition.notify_all();
				}
			}
		}
	}

	bool JobSystem::PopReadyJob(Job& outJob)
	{
		return
			TryPopReady(myHighPriorityJobs, outJob) ||
			TryPopReady(myNormalPriorityJobs, outJob) ||
			TryPopReady(myLowPriorityJobs, outJob);
	}

	bool JobSystem::TryPopReady(std::deque<Job>& aQueue, Job& outJob)
	{
		for (size_t i = 0; i < aQueue.size(); ++i)
		{
			bool ready = std::all_of(aQueue[i].Dependencies.begin(), aQueue[i].Dependencies.end(), [](const auto& aDep)
									 {
										 return aDep.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
									 });

			if (ready)
			{
				outJob = std::move(aQueue[i]);
				aQueue.erase(aQueue.begin() + i);
				return true;
			}
		}
		return false;
	}

	std::deque<Job>& JobSystem::GetQueue(JobPriority aPriority)
	{
		if (aPriority == JobPriority::High) return myHighPriorityJobs;
		if (aPriority == JobPriority::Low) return myLowPriorityJobs;
		return myNormalPriorityJobs;
	}
}
