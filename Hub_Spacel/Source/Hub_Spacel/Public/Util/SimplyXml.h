// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XmlFile.h"
#include "XmlNode.h"

namespace SimplyXml
{
	template<typename T>
	struct FContainer
	{
		FString Tag {};
		TArray<T> Values {};
	};

	struct FWriter
	{
		FWriter(FWriter&&) = default;
		FWriter& operator=(FWriter&&) = default;

		FWriter(FString&& _path) : m_path(std::move(_path)) {};

		~FWriter() { delete m_file; m_file = nullptr; }

		FString m_path{};

		template<class ... Ts>
		void write(Ts &&... _containers)
		{
			FString const fileTemplate = "<?xml version=\"1.0\" encoding=\"UTF - 8\"?>\n<root>\n</root>";
			m_file = new FXmlFile(fileTemplate, EConstructMethod::ConstructFromBuffer);

			if (m_file != nullptr)
			{
				FXmlNode* rootNode{ m_file->GetRootNode() };
				if (rootNode == nullptr) return;

				write_impl(_containers...);

				rootNode->SetContent(m_content);
				m_file->Save(m_path);
			}
		}

	private:
		template<class T, class ... Ts>
		void write_impl(T && _t, Ts&& ... _containers)
		{
			write_impl(std::forward<T>(_t));
			write_impl(std::forward<Ts>(_containers)...);
		}

		template<class T>
		void write_impl(T && _t)
		{
			writeNode(std::forward<T>(_t));
		}

		template<typename T>
		void writeNode(FContainer<T> const& _container)
		{
			ensure(true);
		}

		template<>
		void writeNode(FContainer<FVector_NetQuantize> const& _container)
		{
			for (auto const& loc : _container.Values)
			{
				m_content.Append("<" + _container.Tag + " val=\"" + loc.ToString() + "\"/>\n");
			}
		}

	private:
		FXmlFile* m_file { nullptr };
		FString m_content { "\n" };
	};

	struct FReader
	{
		FReader(FReader&&) = default;
		FReader& operator=(FReader &&) = default;

		FReader(FString&& _path) : m_path(std::move(_path)) {};

		FString m_path {};

		template<class ... Ts>
		void read(Ts &&... _containers)
		{
			if (!m_file.LoadFile(m_path)) return;

			read_impl(std::forward<Ts>(_containers)...);

			m_file.Clear();
		}

	private:
		FXmlFile m_file {};

		template<class T, class ... Ts>
		void read_impl(T&& _t, Ts&& ... _containers)
		{
			read_impl(std::forward<T>(_t));
			read_impl(std::forward<Ts>(_containers)...);
		}

		template<class T>
		void read_impl(T&& _t)
		{
			FXmlNode* rootNode{ m_file.GetRootNode() };
			if (rootNode == nullptr) return;

			TArray<FXmlNode*> const& childrenNodes{ rootNode->GetChildrenNodes() };
			for (auto const* node : childrenNodes)
			{
				if (node != nullptr && node->GetTag() == _t.Tag)
				{
					readNode(_t, node);
				}
			}
		}

		template<typename T>
		void readNode(FContainer<T>& _container, FXmlNode const* _node)
		{
			ensure(true);
		}

		template<>
		void readNode(FContainer<FVector>& _container, FXmlNode const* _node)
		{
			if (_node == nullptr) return;
			FVector location{};
			location.InitFromString(_node->GetAttribute("val"));
			_container.Values.Add(location);
		}

		template<>
		void readNode(FContainer<FVector_NetQuantize>& _container, FXmlNode const* _node)
		{
			if (_node == nullptr) return;
			FVector_NetQuantize location{};
			location.InitFromString(_node->GetAttribute("val"));
			_container.Values.Add(location);
		}
	};
}


