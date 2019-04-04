/*
 * Licensed to Systerel under one or more contributor license
 * agreements. See the NOTICE file distributed with this work
 * for additional information regarding copyright ownership.
 * Systerel licenses this file to you under the Apache
 * License, Version 2.0 (the "License"); you may not use this
 * file except in compliance with the License. You may obtain
 * a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

MACHINE
    translate_browse_path_element_it

SEES
    constants,
    msg_translate_browse_path_bs

DEFINITIONS
    d_variables ==
    elements_to_iterate,
    elements_iterated,
    relativePath,
    relativePathEls
    ;

    d_inv ==
    (
    elements_to_iterate : seq(t_RelativePathElt)  &
    elements_iterated   : seq(t_RelativePathElt)  &
    relativePath        : t_RelativePath_i        &
    relativePathEls     : seq(t_RelativePathElt)  &
    elements_iterated ^ elements_to_iterate = relativePathEls
    )

ABSTRACT_VARIABLES
    d_variables

INVARIANT
    d_inv

INITIALISATION
    d_variables :( d_inv )

OPERATIONS

    p_continue <-- init_iter_relativePath(p_relativePath) =
    PRE
        p_relativePath : t_RelativePath_i
    THEN
        elements_iterated   := {}        ||
        elements_to_iterate := RelativePath_RelativePathElt(p_relativePath) ||
        p_continue          := bool(elements_to_iterate /= {}) ||
        relativePath        := p_relativePath ||
        relativePathEls     := RelativePath_RelativePathElt(p_relativePath)
    END
    ;

    p_continue, p_relativePathElt, p_index <-- continue_iter_relativePath =
    PRE
        elements_to_iterate /= {} &
        size(elements_to_iterate) > 0 &
        relativePathEls = RelativePath_RelativePathElt(relativePath)
    THEN
        ANY l_relativePathElt WHERE
            l_relativePathElt : t_RelativePathElt_i &
            l_relativePathElt : t_RelativePathElt   &
            l_relativePathElt = first(elements_to_iterate)
        THEN
            elements_iterated    := elements_iterated <- l_relativePathElt ||
            elements_to_iterate  := tail(elements_to_iterate)              ||
            p_relativePathElt    := l_relativePathElt                      ||
            p_index              := size(elements_iterated) + 1            ||
            p_continue           := bool(size(elements_to_iterate) > 1)
        END
    END

END
