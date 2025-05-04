#pragma once

#include <ntifs.h>

typedef unsigned long long uint64_t;

constexpr uint64_t PAGE_ENTRY_SIZE = sizeof(uint64_t);
constexpr uint64_t PAGE_MAP_ENTRIES = PAGE_SIZE / PAGE_ENTRY_SIZE;

constexpr uint64_t PMI_BITS = 9; // Page Map Index bits (log2(PAGE_MAP_ENTRIES))

constexpr uint64_t PAGE_MASK_4K = (~0ULL >> 12) & ~((1ULL << (PAGE_SHIFT)) - 1);
constexpr uint64_t PAGE_MASK_2M = (~0ULL >> 12) & ~((1ULL << (PAGE_SHIFT + PMI_BITS)) - 1);
constexpr uint64_t PAGE_MASK_1G = (~0ULL >> 12) & ~((1ULL << (PAGE_SHIFT + PMI_BITS + PMI_BITS)) - 1);


// PML4

union PageMapLevel4Entry {
private:
	uint64_t _raw;

	struct Fields {
		uint64_t			present : 1;
		uint64_t		  writeable : 1;
		uint64_t		user_access : 1;
		uint64_t	  write_through : 1;
		uint64_t	 cache_disabled : 1;
		uint64_t		   accessed : 1;
		uint64_t	      ignored_3 : 1;
		uint64_t			   size : 1; // must be 0
		uint64_t		  ignored_2 : 4;
		uint64_t		   _address : 40;
		uint64_t				cow : 1;
		uint64_t		  ignored_1 : 9;
		uint64_t			swapped : 1;
		uint64_t execution_disabled : 1;
	} fields;

public:
	PageMapLevel4Entry(uint64_t value = 0) : _raw(value) {}

	uint64_t get_address() const {
		return _raw & PAGE_MASK_4K;
	}

	bool present() const {
		return fields.present != 0;
	}
};

static_assert(sizeof(PageMapLevel4Entry) == 8);


// PML3 (Page directory pointer table)

union PageMapLevel3Entry {
private:
	uint64_t _raw;

	struct Fields2 {
		uint64_t		_common_low : 12;
		uint64_t		   _address : 40;
		uint64_t				cow : 1;
		uint64_t		 _ignored_1 : 9;
		uint64_t			swapped : 1;
		uint64_t execution_disabled : 1;
	} fields2;

	struct Fields1 {
		uint64_t			present : 1;
		uint64_t		  writeable : 1;
		uint64_t		user_access : 1;
		uint64_t	  write_through : 1;
		uint64_t	 cache_disabled : 1;
		uint64_t		   accessed : 1;
		uint64_t			  dirty : 1;
		uint64_t			   size : 1;  // 0 means PML2, 1 means 1gb page mapped
		uint64_t			 global : 1;
		uint64_t		 _ignored_2 : 3;
		uint64_t			    pat : 1;
		uint64_t		_reserved_2 : 17;
		uint64_t	  _address_huge : 22;
		uint64_t	   _common_high : 12;
	} fields1;

public:
	PageMapLevel3Entry(uint64_t value = 0) : _raw(value) {}

	uint64_t get_address() const {
		return _raw & PAGE_MASK_4K;
	}

	uint64_t get_huge_address() const {
		return _raw & PAGE_MASK_1G;
	}

	bool present() const {
		return fields1.present != 0;
	}

	bool is_huge() const {
		return fields1.size != 0;
	}
};

static_assert(sizeof(PageMapLevel3Entry) == 8);


// PML2 (Page directory table)

union PageMapLevel2Entry {
private:
	uint64_t _raw;

	struct Fields2 {
		uint64_t		_common_low : 12;
		uint64_t		   _address : 40;
		uint64_t				cow : 1;
		uint64_t		 _ignored_1 : 9;
		uint64_t			swapped : 1;
		uint64_t execution_disabled : 1;
	} fields2;

	struct Fields1 {
		uint64_t			present : 1;
		uint64_t		  writeable : 1;
		uint64_t		user_access : 1;
		uint64_t	  write_through : 1;
		uint64_t	 cache_disabled : 1;
		uint64_t		   accessed : 1;
		uint64_t			  dirty : 1;
		uint64_t			   size : 1;  // 0 means PML1, 1 means 2mb page mapped
		uint64_t			 global : 1;
		uint64_t		 _ignored_2 : 3;
		uint64_t			    pat : 1;
		uint64_t		_reserved_2 : 8;
		uint64_t	 _address_large : 31;
		uint64_t	   _common_high : 12;
	} fields1;

public:
	PageMapLevel2Entry(uint64_t value = 0) : _raw(value) {}

	uint64_t get_address() const {
		return _raw & PAGE_MASK_4K;
	}

	uint64_t get_large_address() const {
		return _raw & PAGE_MASK_2M;
	}

	bool present() const {
		return fields1.present != 0;
	}

	bool is_large() const {
		return fields1.size != 0;
	}
};

static_assert(sizeof(PageMapLevel2Entry) == 8);


// PML1 (Page table)

union PageMapLevel1Entry {
private:
	uint64_t _raw;

	struct Fields {
		uint64_t			present : 1;
		uint64_t		  writeable : 1;
		uint64_t		user_access : 1;
		uint64_t	  write_through : 1;
		uint64_t	 cache_disabled : 1;
		uint64_t		   accessed : 1;
		uint64_t			  dirty : 1;
		uint64_t			   size : 1;
		uint64_t			 global : 1;
		uint64_t		  ignored_2 : 3;
		uint64_t		   _address : 40;
		uint64_t	      ignored_1 : 8;
		uint64_t	  to_be_cleared : 1;
		uint64_t			swapped : 1;
		uint64_t				cow : 1;
		uint64_t execution_disabled : 1;
	} fields;

public:
	PageMapLevel1Entry(uint64_t value = 0) : _raw(value) {}

	uint64_t get_address() const {
		return _raw & PAGE_MASK_4K;
	}

	bool present() const {
		return fields.present != 0;
	}
};

static_assert(sizeof(PageMapLevel1Entry) == 8);


// Virtual Address Structure

union VirtualAddress {
	uint64_t address; // The complete 64-bit address

private:
	struct Fields {
		uint64_t offset : 12;
		uint64_t pml1_i : 9;
		uint64_t pml2_i : 9;
		uint64_t pml3_i : 9;
		uint64_t pml4_i : 9;
		uint64_t _unused0 : 16;
	} fields;

	struct Fields_2m {
		uint64_t offset_2m : 21;
		uint64_t _unused1 : 43;
	} fields_2m;

	struct Fields_1g {
		uint64_t offset_1g : 30;
		uint64_t _unused2 : 34;
	} fields_1g;

public:
	// Constructor for easy initialization
	VirtualAddress(uint64_t addr = 0) : address(addr) {}

	uint64_t get_pml4_index() const {
		return fields.pml4_i;
	}

	uint64_t get_pml3_index() const {
		return fields.pml3_i;
	}

	uint64_t get_pml2_index() const {
		return fields.pml2_i;
	}

	uint64_t get_pml1_index() const {
		return fields.pml1_i;
	}

	uint64_t get_offset() const {
		return fields.offset;
	}

	uint64_t get_offset_2m() const {
		return fields_2m.offset_2m;
	}

	uint64_t get_offset_1g() const {
		return fields_1g.offset_1g;
	}
};
