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
	};
}


