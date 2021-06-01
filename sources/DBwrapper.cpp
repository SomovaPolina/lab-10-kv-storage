//
// Copyright 2020 Your Name Created by polinka on 20.04.2021.
//
#include "DBwrapper.hpp"

#include "iostream"

void rocksdbWrapper::getFamiliesFromBD() {
  rocksdb::Options options;
  rocksdb::Status status = rocksdb::DB::OpenForReadOnly(options, path_, &db_);
  if (!status.ok()) std::cerr << status.ToString() << std::endl;
  db_->ListColumnFamilies(options, path_, &families_);
  delete db_; //закрываю бд
}
void rocksdbWrapper::createDatabase() {
  rocksdb::Options options;
  options.create_if_missing = true;

  rocksdb::Status status = rocksdb::DB::Open(options, path_, &db_);
  if (!status.ok()) std::cerr << status.ToString() << std::endl;

  for (int i = 0; i < familyNum_; ++i) {
    rocksdb::ColumnFamilyHandle* cf;
    status = db_->CreateColumnFamily(rocksdb::ColumnFamilyOptions(),
                                     "family_" + std::to_string(i), &cf);
    assert(status.ok());
    db_->DestroyColumnFamilyHandle(cf);
  }

  delete db_; //удаляю бд из памяти
}

std::vector<std::string> rocksdbWrapper::getFamilyNum() { return families_ ;}

void rocksdbWrapper::pushData() {
  rocksdb::Options options;

  std::vector<rocksdb::ColumnFamilyDescriptor> column_families;

  for (auto& family : families_) {
    column_families.push_back(rocksdb::ColumnFamilyDescriptor(
        family, rocksdb::ColumnFamilyOptions()));
  }

  std::vector<rocksdb::ColumnFamilyHandle*> handles;
  rocksdb::Status status = rocksdb::DB::Open(rocksdb::DBOptions(), path_,
                                             column_families, &handles, &db_);

  assert(status.ok());

  for (size_t i = 0; i < families_.size(); ++i) {
    for (int k = 0; k < columnSize_; ++k) {
      status = db_->Put(rocksdb::WriteOptions(), handles[i],
                        rocksdb::Slice("key_" + std::to_string(k)),
                        rocksdb::Slice("value_" + std::to_string(k)));
      assert(status.ok());
    }
  }
  for (auto handle : handles) {
    status = db_->DestroyColumnFamilyHandle(handle);
    assert(status.ok());
  }

  delete db_;
}

void rocksdbWrapper::migrateDataToMap(std::string logLevel) {
  rocksdb::Options options;
  std::vector<rocksdb::ColumnFamilyHandle*> handles;
  std::map<std::string, std::string> kvStorage;
  std::vector<rocksdb::ColumnFamilyDescriptor> column_families;
  for (const auto& family : families_) {

    column_families.push_back(rocksdb::ColumnFamilyDescriptor(
        family, rocksdb::ColumnFamilyOptions()));

    rocksdb::Status status = rocksdb::DB::OpenForReadOnly(rocksdb::DBOptions(), path_,
                                                          column_families, &handles, &db_);
    assert(status.ok());
    rocksdb::Iterator* it = db_->NewIterator(rocksdb::ReadOptions());

    for (it->SeekToFirst(); it->Valid(); it->Next()) {
      kvStorage[it->key().ToString()] = it->value().ToString();
    }
    hasherObj_.startHashing(family, kvStorage, logLevel);

    kvStorage.clear();
    assert(it->status().ok());
    for (auto& handle : handles) {
      status = db_->DestroyColumnFamilyHandle(handle);
      assert(status.ok());
    }
    delete it;
  }
  delete db_;
}

void rocksdbWrapper::createOutputDatabase() {
  rocksdb::Options options;
  options.create_if_missing = true;
  options.error_if_exists = true;
  rocksdb::Status status = rocksdb::DB::Open(options, path_, &db_);
  if (!status.ok()) std::cerr << status.ToString() << std::endl;

  for (auto const& x : mapa_) {
    if (x.first ==  "default"){
      continue;
    }
    rocksdb::ColumnFamilyHandle* cf;
    status =
        db_->CreateColumnFamily(rocksdb::ColumnFamilyOptions(), x.first, &cf);
    assert(status.ok());
    db_->DestroyColumnFamilyHandle(cf);
  }

  delete db_;

  std::vector<rocksdb::ColumnFamilyDescriptor> column_families;

  for (auto& family : mapa_) {
    column_families.push_back(rocksdb::ColumnFamilyDescriptor(
        family.first, rocksdb::ColumnFamilyOptions()));
  }

  std::vector<rocksdb::ColumnFamilyHandle*> handles;
  status = rocksdb::DB::Open(rocksdb::DBOptions(), path_, column_families,
                             &handles, &db_);

  assert(status.ok());
  for (size_t i = 0;i<mapa_.size();++i) {
    for (auto& kv : mapa_[handles[i]->GetName()]){
      status = db_->Put(rocksdb::WriteOptions(), handles[i],
                        rocksdb::Slice(kv.first),
                        rocksdb::Slice(kv.second));
      assert(status.ok());
    }
  }

  for (auto& handle : handles) {
    status = db_->DestroyColumnFamilyHandle(handle);
    assert(status.ok());
  }
  delete db_;
}