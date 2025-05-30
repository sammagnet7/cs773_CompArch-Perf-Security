# Copyright (c) 2012-2013 ARM Limited
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
from m5.objects.ClockedObject import ClockedObject
from m5.objects.IndexingPolicies import *

class BaseTags(ClockedObject):
    type = 'BaseTags'
    abstract = True
    cxx_header = "mem/cache/tags/base.hh"

    # Get system to which it belongs
    system = Param.System(Parent.any, "System we belong to")

    ghost = Param.Bool(False,"spooky")

    # Get the size from the parent (cache)
    size = Param.MemorySize(Parent.size, "capacity in bytes")
    ghostSize = Param.MemorySize(Parent.ghostSize, "ghost capacity in bytes")



    # Get the block size from the parent (system)
    block_size = Param.Int(Parent.cache_line_size, "block size in bytes")

    # Get the tag lookup latency from the parent (cache)
    tag_latency = Param.Cycles(Parent.tag_latency,
                               "The tag lookup latency for this cache")

    # Get the warmup percentage from the parent (cache)
    warmup_percentage = Param.Percent(Parent.warmup_percentage,
        "Percentage of tags to be touched to warm up the cache")

    sequential_access = Param.Bool(Parent.sequential_access,
        "Whether to access tags and data sequentially")

    # Get indexing policy
    indexing_policy = Param.BaseIndexingPolicy(SetAssociative(),
        "Indexing policy")

    # Set the indexing entry size as the block size
    entry_size = Param.Int(Parent.cache_line_size,
                           "Indexing entry size in bytes")
    
    # Config for Skewed Associative Randomized Caches
    mem_size = Param.MemorySize(Parent.mem_size,"Memory capacity (only for skewed assoc rand caches)")
    randomizedIndexing = Param.Bool(Parent.randomizedIndexing, "Enables Randomized Indexing with QARMA")
    skewedCache = Param.Bool(Parent.skewedCache, "Enables skewed cache design")
    numSkews = Param.Unsigned(Parent.numSkews, "Number of skews (only for skewed assoc rand caches)")



class BaseSetAssoc(BaseTags):
    type = 'BaseSetAssoc'
    cxx_header = "mem/cache/tags/base_set_assoc.hh"

    # Get the cache associativity
    assoc = Param.Int(Parent.assoc, "associativity")
    ghostAssoc = Param.Int(Parent.ghostAssoc, "ghost associativity")

    # Get replacement policy from the parent (cache)
    replacement_policy = Param.BaseReplacementPolicy(
        Parent.replacement_policy, "Replacement policy")

class SectorTags(BaseTags):
    type = 'SectorTags'
    cxx_header = "mem/cache/tags/sector_tags.hh"

    # Get the cache associativity
    assoc = Param.Int(Parent.assoc, "associativity")

    # Number of sub-sectors (data blocks) per sector
    num_blocks_per_sector = Param.Int(1, "Number of sub-sectors per sector");

    # The indexing entry now is a sector block
    entry_size = Parent.cache_line_size * Self.num_blocks_per_sector

    # Get replacement policy from the parent (cache)
    replacement_policy = Param.BaseReplacementPolicy(
        Parent.replacement_policy, "Replacement policy")

class CompressedTags(SectorTags):
    type = 'CompressedTags'
    cxx_header = "mem/cache/tags/compressed_tags.hh"

    # Maximum number of compressed blocks per tag
    max_compression_ratio = Param.Int(2,
        "Maximum number of compressed blocks per tag.")

    # We simulate superblock as sector blocks
    num_blocks_per_sector = Self.max_compression_ratio

    # We virtually increase the number of data blocks per tag by multiplying
    # the cache size by the compression ratio
    size = Parent.size * Self.max_compression_ratio

class FALRU(BaseTags):
    type = 'FALRU'
    cxx_class = 'FALRU'
    cxx_header = "mem/cache/tags/fa_lru.hh"

    min_tracked_cache_size = Param.MemorySize("128kB", "Minimum cache size for"
                                              " which we track statistics")

    # This tag uses its own embedded indexing
    indexing_policy = NULL


class VwayTags(BaseTags):
    type = 'VwayTags'
    cxx_header = "mem/cache/tags/vway_tags.hh"
    cxx_class = "VwayTags"

    #Get the TDR (tag-dat ratio) for over-provisioning tags
    TDR = Param.Float(Parent.TDR, "Tag-to-Data ratio")

    # Get the cache associativity from the parent (cache)
    cache_assoc = Param.Int(Parent.assoc, "cache associativity")
    
    # Get the tag-store associativity
    assoc = Param.Int(Parent.assoc, "tag-store associativity")

    # Get the cache size from the parent (cache)
    cache_size = Param.MemorySize(Parent.size, "cache capacity in bytes")

    # Get the tag-store size
    #size = Param.MemorySize(Parent.size, "tag-store capacity in bytes")
    
    # Get local replacement policy from the parent (cache)
    replacement_policy = Param.BaseReplacementPolicy(
        Parent.replacement_policy, "Replacement policy")
    
    #Get Flags for handling local-conflicts [Skewed Randomized V-way cache]
    p2_on_conflict = Param.Bool(Parent.p2_on_conflict, "Enables 2 Power-of-Choices Replacement on Local Conflict")
    cuckoo_on_conflict = Param.Bool(Parent.cuckoo_on_conflict, "Enables Cuckoo based Re-fill on Local Conflict")

    #Data-Replacement policy
    data_repl_policy = Param.String("random","Can be [random,reuse]")
