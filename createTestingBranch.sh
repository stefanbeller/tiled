#!/bin/bash

CheckGitIsSafe()
{
    if [ "$(git symbolic-ref HEAD)" != "refs/heads/testing" ] ; then
        echo "Not on testing branch"
        exit
    fi
}

GetFirstWordOfLastCommit()
{
    FirstWordOfLastCommit=$(git log --oneline |head -n1|awk '{print $2}')
}

ResetHistory()
{
    # HEAD back in history until we are on non-Merge nodes.
    # That is hopefully the commit which adds this script
    GetFirstWordOfLastCommit
    while [ "$FirstWordOfLastCommit" == "Merge" ] ; do
        git reset --hard HEAD^
        GetFirstWordOfLastCommit
    done
    # Now that only one commit (this script) is ontop of an old mainline
    # rebase to have all mainline stuff definitly included
    git rebase mainline/master
}

MergeOneBranch()
{
    #take $branch as parameter
    gitsuccess=0
    makesuccess=0
    latestcommit=$(git rev-parse HEAD)

    git merge --no-edit $branch
    gitsuccess=$?

    make
    makesuccess=$?
    let makecounter=makecounter+1
    if [ "$makesuccess" -ne "0" ]; then
        git merge --abort
        git reset --hard ${latestcommit}
        failedbranches+=($branch)
    else
        if [ "$gitsuccess" -ne "0" ] ; then
            git commit -a --no-edit
        fi
        let successcounter=successcounter+1
        successfulbranches+=($branch)
    fi
    let overallcounter=overallcounter+1
}

MergeBranches()
{
    for branch in "${branches[@]}"; do
        echo $branch
        MergeOneBranch
    done
}

MergePriorityBranches()
{
    branches=()
    successfulbranches=()
    failedbranches=()
    NumBranches=${#PriorityBranches[@]}
    for (( j=0; j<${NumBranches}; j++ )) ; do
        branch="${PriorityBranches[$j]}"
        branches+=($branch)
    done
    MergeBranches
}

MergeAllBranches()
{
    branches=()
    successfulbranches=()
    failedbranches=()
    eval "$(git for-each-ref --shell --format='branches+=(%(refname))' refs/)"
    MergeBranches
}

Initialize()
{
    successcounter=0
    overallcounter=0
    makecounter=0
    PriorityBranches=()
    StartTime=$(date +%s)  # in unix time, seconds
}

PrintInformation()
{
    EndTime=$(date +%s)
    DiffTime=$(( $EndTime - $StartTime ))
    echo "It took $DiffTime seconds!"
    echo "I considered merging $overallcounter branches,"
    echo "$successcounter branches have been merged successfully."
    echo "I needed to compile $makecounter branches."

    NumSucBranches=${#successfulbranches[@]}
    echo "These $NumSucBranches branches were successfully merged and compiled:"
    for (( j=0; j<${NumSucBranches}; j++ )) ; do
        echo "${successfulbranches[$j]}"
    done
    NumFailBranches=${#failedbranches[@]}
    echo "These $NumFailBranches branches had problems being merged:"
    for (( j=0; j<${NumFailBranches}; j++ )) ; do
        echo "${failedbranches[$j]}"
    done
}

Initialize
CheckGitIsSafe
ResetHistory
echo "Please change priority branches if any in the createTestingBranch.conf file"
read -p "Press [Enter] when you're done:"

if [ -e ./createTestingBranch.conf ] ; then
    source createTestingBranch.conf
fi

NumBranches=${#PriorityBranches[@]}
if [ "$NumBranches" != "0" ] ; then
    MergePriorityBranches
else
    echo "createTestingBranch.conf does not exist or did not define the PriorityBranches"
    read -p "Continue merging all branches? [Yn]" decision
    if [ "$decision" == "" ] ; then
        MergeAllBranches  # This will try to merge all available branches configured in git
    fi
fi
PrintInformation

read -p "(Forced-)Push changes? [Yn]" decision
if [ "$decision" == "" ] ; then
    git push -f origin testing
fi
