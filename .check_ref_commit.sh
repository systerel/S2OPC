#!/bin/bash

# Licensed to Systerel under one or more contributor license
# agreements. See the NOTICE file distributed with this work
# for additional information regarding copyright ownership.
# Systerel licenses this file to you under the Apache
# License, Version 2.0 (the "License"); you may not use this
# file except in compliance with the License. You may obtain
# a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.

# This script is used in order to check MR and Issue consistency before merging.
# It has knowledge of the MR related (See parameter $3) and thus can check that:
# - The related MR is Open, and has the correct milestone if present (See CI variable SOPC_CURRENT_MILESTONE),
# - Then checks for each commit of the MR:
#   - The referred ticket in commit is linked to the expected MR
#   - The referred ticket in commit is opened
#   - The referred ticket in commit has exaclty one label among 'Enhancement|Defect|New Feature'
#   - The referred ticket in commit has the expected milestone
# - Moreover, the script will fail if the MR contains no new commit

function usage() {
    echo "$0 : Check the commits and related MR."
    echo "Usage:"
    echo "    $0 [-v] [-o] <milestone> <target_branch> <ref_mr> <url>"
    echo "With (note that the arguments and options are expected in this very order):"
    echo "  -v              : Verbose mode"
    echo "  -o              : Also check that the ticket is open"
    echo "  <milestone>     : the expected MILESTONE  (See CI variable SOPC_CURRENT_MILESTONE)"
    echo "  <target_branch> : the expected target merge branch  (See GITLAB variable CI_MERGE_REQUEST_TARGET_BRANCH_NAME)"
    echo "  <ref_mr>        : the related merge request (See GITLAB variable CI_MERGE_REQUEST_IID)"
    echo "  <url>           : the REST API for GITLaB access (See GITLAB variables CI_API_V4_URL and CI_PROJECT_ID)"
}

VERBOSE=false
CHECK_OPEN=false
[[ $1 =~ -(-)?h(elp)? ]] && usage && exit 1
[[ $1 == "-v" ]] && shift && VERBOSE=true
[[ $1 == "-o" ]] && shift && CHECK_OPEN=true

regex='([0-9a-f]+)[[:blank:]]Ticket[[:blank:]]*#([0-9]+)[[:blank:]]*:.*'
req_labels='Enhancement|Defect|New Feature'
CURRENT_MILESTONE=$1
TARGET_BRANCH=$2
REF_MR=$3
URL=$4

[[ -z ${CURRENT_MILESTONE} ]] && usage && exit 1

SHA=$(git log -1 --oneline | cut -d ' ' -f 1)
echo "Checking commits ${TARGET_BRANCH}..${SHA}"

echo "CURRENT_MILESTONE=${CURRENT_MILESTONE}"
echo "TARGET_BRANCH=${TARGET_BRANCH}"
echo "REF_MR=${REF_MR}"
echo "URL=${URL}"

echo -n "Check CURL..."
command -v curl || exit 1

function URL_REQUEST() {
    local path="$1"
    local varname="$2"
    local cmd=(curl -s "${URL}/${path}")
    ${VERBOSE} && echo cmd: "${cmd[@]}"
    data=$("${cmd[@]}")
    eval "$varname"="$(printf %q "${data}")"
}

function JSON_EXTRACT() {
    local SRC="$1"
    shift
    echo "${SRC}" |  python -c "import json,sys;obj=json.load(sys.stdin);print ($*)"
}

HAS_ERRORS=false
HAS_COMMITS=false

ERRFILE=$(mktemp)
COMMITFILE=$(mktemp)
rm -f "${ERRFILE}" "${COMMITFILE}"

# Read MR infos
URL_REQUEST "merge_requests/${REF_MR}" MR_INFO
MR_ID=$(JSON_EXTRACT "${MR_INFO}" 'obj["iid"]')
MR_STATE=$(JSON_EXTRACT "${MR_INFO}" 'obj[u"state"]')
MR_DRAFT=$(JSON_EXTRACT "${MR_INFO}" 'obj[u"draft"]')
MR_LABELS=$(JSON_EXTRACT "${MR_INFO}" 'obj[u"labels"]')


${VERBOSE} && echo "MR_STATE=$MR_STATE, MR_DRAFT=$MR_DRAFT, MR_LABELS=$MR_LABELS "

# Check MR Milestone (only if present)
MR_MILESTONE=$(JSON_EXTRACT "${MR_INFO}" 'obj[u"milestone"][u"title"]')
if [[ "${MR_MILESTONE}" == "" ]];then
    ${VERBOSE} && echo "MR !${MR_ID} has no milestone"
elif [[ "${MR_MILESTONE}" != "${CURRENT_MILESTONE}" ]];then
    echo "[Error] MR !${MR_ID} has invalid milestone (${MR_MILESTONE} found instead of '${CURRENT_MILESTONE}')" |tee -a "${ERRFILE}"
else
    ${VERBOSE} && echo "MR !${MR_ID} has a valid milestone (${MR_MILESTONE})"
fi

# Check MR status
if [[ ${MR_STATE} != opened ]];then
    echo "[Error] MR !${MR_ID} has invalid status ('${MR_STATE}' found instead of 'opened')" |tee -a "${ERRFILE}"
else
    ${VERBOSE} && echo "MR !${MR_ID} has a valid status '${MR_STATE}'"
fi


git log --no-merges HEAD "^${TARGET_BRANCH}" --oneline --no-decorate | while read -r line || [[ -n ${line} ]];
do
    if [[ ${line} =~ ${regex} ]];then
        COMMIT_ID=${BASH_REMATCH[1]}
        TICKET_ID=${BASH_REMATCH[2]}
        NAME="Ticket #${TICKET_ID} (SHA ${COMMIT_ID})"
        ${VERBOSE} && echo "------------"
        # Skip tickets that have already been checked
        grep -q "#${TICKET_ID}" "${COMMITFILE}" && continue
        echo "Processing ${NAME}..."
        echo "#${TICKET_ID}" >> "${COMMITFILE}"
        
        URL_REQUEST "issues/${TICKET_ID}" ISSUE
        ${VERBOSE} && echo "ISSUE=$ISSUE"
        NB_LABELS=$(JSON_EXTRACT "${ISSUE}" 'len([s for s in obj[u"labels"] if s in ["Enhancement","Defect","New Feature"]])')
        
        if [[ ${NB_LABELS} != 1 ]]; then
            echo "[Error] ${NAME} has invalid labels count (${NB_LABELS} found instead of 1 in '${req_labels}')" |tee -a "${ERRFILE}"
        else
            ${VERBOSE} && echo "${NAME} has valid labels count '${NB_LABELS}'"
        fi
         
        # Check milestone   
        TICKET_MILESTONE=$(JSON_EXTRACT "${ISSUE}" '(obj[u"milestone"][u"title"] if obj[u"milestone"] else "None")')
        if [[ "${TICKET_MILESTONE}" != "${CURRENT_MILESTONE}" ]];then
            echo "[Error] ${NAME} has invalid milestone ('${TICKET_MILESTONE}' found instead of '${CURRENT_MILESTONE}')" |tee -a "${ERRFILE}"
        else
            ${VERBOSE} && echo "${NAME} has a valid milestone (${TICKET_MILESTONE})"
        fi
        
        if ${CHECK_OPEN} ; then
            STATE=$(JSON_EXTRACT "${ISSUE}" 'obj[u"state"]')
            if [[ ${STATE} != "opened" ]]; then
                echo "[Error] ${NAME} has invalid status ('${STATE}' found instead of 'opened')" |tee -a "${ERRFILE}"
            else
                ${VERBOSE} && echo "${NAME} has valid status '${STATE}'"
            fi
        fi

        # Retreive list of MR
        URL_REQUEST "issues/${TICKET_ID}/related_merge_requests" MRS
        MR_RID=$(JSON_EXTRACT "${MRS}" '["!%s"%o["iid"] for o in obj]')
        MR_RID_OK=$(JSON_EXTRACT "${MRS}" "${MR_ID}"' in [o["iid"] for o in obj]')
                    
        # Check MR validity
        if [[ ${MR_RID_OK} != "True" ]] ; then
            echo "[Error] Expected MR !${MR_ID} not found in related MR of ${NAME} (found ${MR_RID})" |tee -a "${ERRFILE}"
        else
            ${VERBOSE} && echo "${NAME} has valid MR '${MR_RID}'"
        fi
    else
        echo "[Error] Commit header has invalid format: '${line}'"
        HAS_ERRORS=true
    fi
done
if [[ -f ${ERRFILE} ]];then
    HAS_ERRORS=true
    echo "---------------"
    echo "Errors summary:"
    cat "${ERRFILE}" >&2
    
fi

if [[ -f ${COMMITFILE} ]];then
    HAS_COMMITS=true
fi
echo HAS_COMMITS=$HAS_COMMITS, HAS_ERRORS=$HAS_ERRORS

rm -f "${COMMITFILE}" "${ERRFILE}"

$HAS_ERRORS && echo "Error(s) found" && exit 1
(! $HAS_COMMITS) && echo "No commits found" && exit 2
echo "OK"

