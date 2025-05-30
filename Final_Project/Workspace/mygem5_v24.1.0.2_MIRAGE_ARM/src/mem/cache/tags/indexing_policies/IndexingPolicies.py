# -*- mode:python -*-
# Copyright (c) 2024 Arm Limited
# All rights reserved.
#
# The license below extends only to copyright in the software and shall
# not be construed as granting a license to any other intellectual
# property including but not limited to intellectual property relating
# to a hardware implementation of the functionality of the software
# licensed hereunder.  You may use the software subject to the license
# terms below provided that you ensure that this notice is replicated
# unmodified and in its entirety in all distributions of the software,
# modified or unmodified, in source code or in binary form.
#
# Copyright (c) 2018 Inria
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met: redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer;
# redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution;
# neither the name of the copyright holders nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

from m5.params import *
from m5.proxy import *
from m5.SimObject import SimObject


class TaggedIndexingPolicy(SimObject):
    type = "TaggedIndexingPolicy"
    abstract = True
    cxx_class = "gem5::IndexingPolicyTemplate<gem5::TaggedTypes>"
    cxx_header = "mem/cache/tags/tagged_entry.hh"
    cxx_template_params = ["class Types"]

    # Get the associativity
    assoc = Param.Int(Parent.assoc, "associativity")

class BaseIndexingPolicy(SimObject):
    type = "BaseIndexingPolicy"
    abstract = True
    cxx_class = "gem5::IndexingPolicyTemplate<gem5::AddrTypes>"
    cxx_header = "mem/cache/tags/indexing_policies/base.hh"
    cxx_template_params = ["class Types"]

    # Get the associativity
    assoc = Param.Int(Parent.assoc, "associativity")


class SetAssociative(BaseIndexingPolicy):
    type = "SetAssociative"
    cxx_class = "gem5::SetAssociative"
    cxx_header = "mem/cache/tags/indexing_policies/set_associative.hh"

    # Get the size from the parent (cache)
    size = Param.MemorySize(Parent.size, "capacity in bytes")

    # Get the entry size from the parent (tags)
    entry_size = Param.Int(Parent.entry_size, "entry size in bytes")


class SkewedAssociative(BaseIndexingPolicy):
    type = "SkewedAssociative"
    cxx_class = "gem5::SkewedAssociative"
    cxx_header = "mem/cache/tags/indexing_policies/skewed_associative.hh"

    # Get the size from the parent (cache)
    size = Param.MemorySize(Parent.size, "capacity in bytes")

    # Get the entry size from the parent (tags)
    entry_size = Param.Int(Parent.entry_size, "entry size in bytes")

class SkewedAssocRand(TaggedIndexingPolicy):
    type = 'SkewedAssocRand'
    cxx_class = 'gem5::SkewedAssocRand' # MODIFIED Add proper scope
    cxx_header = "mem/cache/tags/indexing_policies/skewed_assoc_randomized.hh"

    mem_size = Param.MemorySize(Parent.size,"Memory capacity")
    # Get the entry size from the parent (tags)
    entry_size = Param.Int(Parent.entry_size, "entry size in bytes")
    randomizedIndexing = Param.Bool(Parent.randomizedIndexing, "Enables Randomized Indexing with PRINCE")
    skewedCache = Param.Bool(Parent.skewedCache, "Enables skewed cache design")
    numSkews = Param.Unsigned(Parent.numSkews, "Number of skews")